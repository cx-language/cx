#include "lex.h"
#include <cctype>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)
#include "../ast/token.h"
#include "../support/utility.h"
#include "parse.h"

using namespace cx;

Lexer::Lexer(llvm::MemoryBufferRef input)
: buffer(input), firstLocation(input.getBufferIdentifier().data(), 1, 0), lastLocation(input.getBufferIdentifier().data(), 1, 0) {
    currentFilePosition = buffer.getBufferStart() - 1;
}

const char* Lexer::getFilePath() const {
    return buffer.getBufferIdentifier().data();
}

Location Lexer::getCurrentLocation() const {
    return Location(getFilePath(), firstLocation.line, firstLocation.column);
}

char Lexer::readChar() {
    char ch = *++currentFilePosition;
    if (ch != '\n') {
        lastLocation.column++;
    } else {
        lastLocation.line++;
        lastLocation.column = 0;
    }
    return ch;
}

void Lexer::unreadChar(char ch) {
    if (ch != '\n') {
        lastLocation.column--;
    } else {
        lastLocation.line--;
        // lastLocation.column can be left as is because the next readChar() call will reset it anyways.
    }
    currentFilePosition--;
}

void Lexer::readBlockComment(Location startLocation) {
    int nestLevel = 1;

    while (true) {
        char ch = readChar();

        if (ch == '*') {
            char next = readChar();

            if (next == '/') {
                nestLevel--;
                if (nestLevel == 0) return;
            } else {
                unreadChar(next);
            }
        } else if (ch == '/') {
            char next = readChar();

            if (next == '*') {
                nestLevel++;
            } else {
                unreadChar(next);
            }
        } else if (ch == '\0') {
            unreadChar(ch);
            REPORT_ERROR(startLocation, "unterminated block comment");
            break;
        }
    }
}

Token Lexer::nextToken() {
    firstLocation.line = lastLocation.line;
    firstLocation.column = lastLocation.column;

    if (pendingInterpStart) {
        pendingInterpStart = false;
        readChar(); // Consume '$'.
        if (pendingInterpBraceForm) readChar(); // Consume '{'.
        LexFrame frame;
        frame.isCode = true;
        frame.codeFrame.braceForm = pendingInterpBraceForm;
        frameStack.push_back(frame);
        return Token(Token::InterpStart, getCurrentLocation(), pendingInterpBraceForm ? "${" : "$");
    }

    if (!frameStack.empty() && frameStack.back().isCode) return lexCodeToken();
    if (!frameStack.empty()) return lexStringResume();
    return lexToken();
}

Token Lexer::lexCodeToken() {
    size_t frameIndex = frameStack.size() - 1;

    if (!frameStack[frameIndex].codeFrame.braceForm && frameStack[frameIndex].codeFrame.firstTokenDone) {
        frameStack.pop_back();
        frameStack.back().stringFrame.contentBegin = currentFilePosition + 1;
        return Token(Token::InterpEnd, getCurrentLocation(), "$");
    }

    if (frameStack[frameIndex].codeFrame.braceForm) {
        char ch = readChar();

        if (ch == '}') {
            if (frameStack[frameIndex].codeFrame.braceDepth == 0) {
                frameStack.pop_back();
                frameStack.back().stringFrame.contentBegin = currentFilePosition + 1;
                return Token(Token::InterpEnd, getCurrentLocation(), "}");
            }
            frameStack[frameIndex].codeFrame.braceDepth--;
            unreadChar(ch);
        } else {
            if (ch == '{') frameStack[frameIndex].codeFrame.braceDepth++;
            unreadChar(ch);
        }
    }

    Token token = lexToken();

    if (token.kind == Token::None) {
        ERROR(getCurrentLocation(), "unterminated interpolation, expected '}'");
    }

    if (!frameStack[frameIndex].codeFrame.braceForm) frameStack[frameIndex].codeFrame.firstTokenDone = true;
    return token;
}

Token Lexer::lexStringResume() {
    StringFrame& stringFrame = frameStack.back().stringFrame;

    while (true) {
        auto ch = readChar();

        if (ch == '\0') {
            ERROR(getCurrentLocation(), "unterminated string literal");
        }

        if (ch == stringFrame.delimiter) {
            const char* chunkEnd = currentFilePosition;
            const char* chunkBegin = stringFrame.contentBegin;
            frameStack.pop_back();

            if (chunkEnd > chunkBegin) {
                return Token(Token::StringLiteral, getCurrentLocation(), llvm::StringRef(chunkBegin, chunkEnd - chunkBegin));
            }
            return nextToken();
        }

        if (ch == '\\') {
            if (readChar() == '\0') {
                ERROR(getCurrentLocation(), "unterminated string literal");
            }
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            Location newlineLocation = firstLocation;
            newlineLocation.column += currentFilePosition - stringFrame.quotePos;
            ERROR(newlineLocation, "newline inside string literal");
        }

        if (stringFrame.delimiter == '"' && ch == '$') {
            char next = readChar();

            if (next != '$' && next != '{' && !std::isalpha(next) && next != '_') {
                unreadChar(next);
                continue;
            }

            if (next == '$') continue;

            const char* dollarPos = currentFilePosition - 1;
            // Interpolation trigger: unread back to '$' so InterpStart
            // consumption stays uniform, then suspend with a flag.
            unreadChar(next);
            unreadChar('$');
            pendingInterpStart = true;
            pendingInterpBraceForm = (next == '{');

            if (dollarPos > stringFrame.contentBegin) {
                return Token(Token::StringLiteral, getCurrentLocation(), llvm::StringRef(stringFrame.contentBegin, dollarPos - stringFrame.contentBegin));
            }
            return nextToken();
        }
    }
}

Token Lexer::readQuotedLiteral(char delimiter, Token::Kind literalKind) {
    const char* begin = currentFilePosition;
    const char* end = begin + 2;
    bool escape = false;

    while (true) {
        auto ch = readChar();

        if (ch == '\0') {
            ERROR(getCurrentLocation(), "unterminated " << toString(literalKind));
        }

        if (escape) {
            escape = false;
        } else if (ch == delimiter) {
            break;
        } else if (ch == '\\') {
            escape = true;
        } else if (ch == '\n' || ch == '\r') {
            Location newlineLocation = firstLocation;
            newlineLocation.column += end - begin - 1;
            ERROR(newlineLocation, "newline inside " << toString(literalKind));
        } else if (delimiter == '"' && ch == '$') {
            char next = readChar();

            if (next == '$') {
                end += 2;
                continue;
            }

            if (next != '{' && !std::isalpha(next) && next != '_') {
                unreadChar(next);
                end++;
                continue;
            }

            // Interpolation trigger: suspend with a flag and return the
            // chunk before '$'. Chunks exclude quotes; the parser builds
            // them without quote-stripping (unlike plain literals below).
            const char* dollarPos = currentFilePosition - 1;
            unreadChar(next);
            unreadChar('$');
            pendingInterpStart = true;
            pendingInterpBraceForm = (next == '{');

            LexFrame frame;
            frame.isCode = false;
            frame.stringFrame.delimiter = delimiter;
            frame.stringFrame.quotePos = begin;
            frameStack.push_back(frame);

            if (dollarPos > begin + 1) {
                return Token(literalKind, getCurrentLocation(), llvm::StringRef(begin + 1, dollarPos - (begin + 1)));
            }
            return nextToken();
        }

        end++;
    }

    return Token(literalKind, getCurrentLocation(), llvm::StringRef(begin, end - begin));
}

Token Lexer::readNumber() {
    const char* const begin = currentFilePosition;
    const char* end = begin + 1;
    bool isFloat = false;
    bool sawSeparator = false;
    bool sawNonSeparator = false;
    bool sawExponent = false;
    uint64_t intValue = *begin - '0';
    char ch = readChar();

    auto appendDigit = [&](uint64_t digit, uint64_t base) {
        if (intValue > (std::numeric_limits<uint64_t>::max() - digit) / base) {
            ERROR(firstLocation, "integer literal is too large");
        }
        intValue = intValue * base + digit;
    };

    switch (ch) {
    case 'b':
        if (begin[0] != '0') goto end;
        end++;
        while (true) {
            ch = readChar();
            if (ch == '0' || ch == '1') {
                appendDigit(ch == '1', 2);
                sawNonSeparator = true;
                end++;
                continue;
            } else if (ch == '_') {
                end++;
                continue;
            }
            if (std::isalnum(ch)) ERROR(lastLocation, "invalid digit '" << ch << "' in binary literal");
            if (end == begin + 2 || !sawNonSeparator) ERROR(firstLocation, "binary literal must have at least one digit after '0b'");
            goto end;
        }
        break;
    case 'o':
        if (begin[0] != '0') goto end;
        end++;
        while (true) {
            ch = readChar();
            if (ch >= '0' && ch <= '7') {
                appendDigit(ch - '0', 8);
                sawNonSeparator = true;
                end++;
                continue;
            } else if (ch == '_') {
                end++;
                continue;
            }
            if (std::isalnum(ch)) ERROR(lastLocation, "invalid digit '" << ch << "' in octal literal");
            if (end == begin + 2 || !sawNonSeparator) ERROR(firstLocation, "octal literal must have at least one digit after '0o'");
            goto end;
        }
        break;
    default:
        if (std::isdigit(ch) && begin[0] == '0') {
            ERROR(firstLocation, "numbers cannot start with 0[0-9], use 0o prefix for octal literal");
        }

        while (true) {
            if (ch == '.' && !isFloat) {
                if (sawSeparator) ERROR(firstLocation, "float literals cannot contain separators");
                isFloat = true;
            } else if ((ch == 'e' || ch == 'E') && !sawExponent) {
                if (sawSeparator) ERROR(firstLocation, "float literals cannot contain separators");
                end++;
                ch = readChar();
                if (ch == '+' || ch == '-') {
                    end++;
                    ch = readChar();
                }
                if (!std::isdigit(ch)) ERROR(firstLocation, "float literal exponent must have at least one digit");
                isFloat = true;
                sawExponent = true;
                end++;
                ch = readChar();
                continue;
            } else if (std::isdigit(ch)) {
                // Only add to the integer value if we're not a floating-point
                // value, otherwise simply continue to the next character
                if (!isFloat) {
                    appendDigit(ch - '0', 10);
                }
            } else if (ch == '_') {
                if (isFloat) ERROR(firstLocation, "float literals cannot contain separators");
                sawSeparator = true;
            } else {
                goto end;
            }
            end++;
            ch = readChar();
        }
        break;
    case 'x':
        if (begin[0] != '0') goto end;
        end++;
        int lettercase = 0; // 0 -> not set yet, >0 -> uppercase, <0 -> lowercase
        while (true) {
            ch = readChar();

            if (std::isdigit(ch)) {
                appendDigit(ch - '0', 16);
                sawNonSeparator = true;
                end++;
            } else if (ch == '_') {
                end++;
            } else if (ch >= 'a' && ch <= 'f') {
                if (lettercase > 0) ERROR(lastLocation, "mixed letter case in hex literal");
                appendDigit(ch - 'a' + 10, 16);
                sawNonSeparator = true;
                end++;
                lettercase = -1;
            } else if (ch >= 'A' && ch <= 'F') {
                if (lettercase < 0) ERROR(lastLocation, "mixed letter case in hex literal");
                appendDigit(ch - 'A' + 10, 16);
                sawNonSeparator = true;
                end++;
                lettercase = 1;
            } else {
                if (std::isalnum(ch)) ERROR(lastLocation, "invalid digit '" << ch << "' in hex literal");
                if (end == begin + 2 || !sawNonSeparator) ERROR(firstLocation, "hex literal must have at least one digit after '0x'");
                goto end;
            }
        }
        break;
    }

end:
    ASSERT(begin != end);
    if (end[-1] == '.') {
        // Exclude the trailing '.' so it's lexed separately (e.g. member access `0.foo`, ranges `0..10`).
        // Rewind explicitly instead of unreading: the terminator may be a newline, whose column unreadChar can't restore.
        end--;
        currentFilePosition = end - 1;
        lastLocation = Location(getFilePath(), firstLocation.line, firstLocation.column + (end - begin) - 1);
        isFloat = false;
    } else {
        unreadChar(ch);
    }

    if (isFloat) return Token(Token::FloatLiteral, getCurrentLocation(), llvm::StringRef(begin, end - begin));
    return Token(getCurrentLocation(), intValue);
}

static const llvm::StringMap<Token::Kind> keywords = {
    {"break", Token::Break},
    {"case", Token::Case},
    {"const", Token::Const},
    {"continue", Token::Continue},
    {"default", Token::Default},
    {"defer", Token::Defer},
    {"do", Token::Do},
    {"else", Token::Else},
    {"enum", Token::Enum},
    {"extern", Token::Extern},
    {"false", Token::False},
    {"for", Token::For},
    {"if", Token::If},
    {"import", Token::Import},
    {"in", Token::In},
    {"interface", Token::Interface},
    {"null", Token::Null},
    {"private", Token::Private},
    {"public", Token::Public},
    {"return", Token::Return},
    {"sizeof", Token::Sizeof},
    {"struct", Token::Struct},
    {"switch", Token::Switch},
    {"then", Token::Then},
    {"this", Token::This},
    {"true", Token::True},
    {"undefined", Token::Undefined},
    {"var", Token::Var},
    {"while", Token::While},
    {"#if", Token::HashIf},
    {"#else", Token::HashElse},
    {"#endif", Token::HashEndif},
};

Token Lexer::lexToken() {
    while (true) {
        char ch = readChar();
        firstLocation.line = lastLocation.line;
        firstLocation.column = lastLocation.column;

        switch (ch) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            break; // skip whitespace
        case '/':
            ch = readChar();
            if (ch == '/') {
                // comment until end of line
                while (true) {
                    char ch = readChar();
                    if (ch == '\n') break;
                    if (ch == '\0') goto end;
                }
            } else if (ch == '*') {
                readBlockComment(firstLocation);
            } else if (ch == '=') {
                return Token(Token::SlashEqual, getCurrentLocation());
            } else {
                unreadChar(ch);
                return Token(Token::Slash, getCurrentLocation());
            }
            break;
        case '+':
            ch = readChar();
            if (ch == '+') return Token(Token::Increment, getCurrentLocation());
            if (ch == '=') return Token(Token::PlusEqual, getCurrentLocation());
            unreadChar(ch);
            return Token(Token::Plus, getCurrentLocation());
        case '-':
            ch = readChar();
            if (ch == '-') return Token(Token::Decrement, getCurrentLocation());
            if (ch == '=') return Token(Token::MinusEqual, getCurrentLocation());
            if (ch == '>') {
                // '->' was the lambda arrow before it was changed to '=>', and C
                // programmers reach for it for member access; recover as a fat
                // arrow so parsing continues and only this error is reported.
                REPORT_ERROR(getCurrentLocation(), "unexpected '->', use '=>' for lambdas or '.' for member access");
                return Token(Token::FatArrow, getCurrentLocation());
            }
            unreadChar(ch);
            return Token(Token::Minus, getCurrentLocation());
        case '*':
            ch = readChar();
            if (ch == '=') return Token(Token::StarEqual, getCurrentLocation());
            unreadChar(ch);
            return Token(Token::Star, getCurrentLocation());
        case '%':
            ch = readChar();
            if (ch == '=') return Token(Token::ModuloEqual, getCurrentLocation());
            if (ch == '%') return Token(Token::PositiveModulo, getCurrentLocation());
            unreadChar(ch);
            return Token(Token::Modulo, getCurrentLocation());
        case '<':
            ch = readChar();
            if (ch == '=') return Token(Token::LessOrEqual, getCurrentLocation());
            if (ch == '<') {
                ch = readChar();
                if (ch == '=') return Token(Token::LeftShiftEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::LeftShift, getCurrentLocation());
            }
            unreadChar(ch);
            return Token(Token::Less, getCurrentLocation());
        case '>':
            ch = readChar();
            if (ch == '=') return Token(Token::GreaterOrEqual, getCurrentLocation());
            if (ch == '>') {
                ch = readChar();
                if (ch == '=') return Token(Token::RightShiftEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::RightShift, getCurrentLocation());
            }
            unreadChar(ch);
            return Token(Token::Greater, getCurrentLocation());
        case '=':
            ch = readChar();
            if (ch == '=') {
                return Token(Token::Equal, getCurrentLocation());
            }
            if (ch == '>') {
                return Token(Token::FatArrow, getCurrentLocation());
            }
            unreadChar(ch);
            return Token(Token::Assignment, getCurrentLocation());
        case '!':
            ch = readChar();
            if (ch == '=') {
                return Token(Token::NotEqual, getCurrentLocation());
            }
            unreadChar(ch);
            return Token(Token::Not, getCurrentLocation());
        case '&':
            ch = readChar();
            if (ch == '&') {
                ch = readChar();
                if (ch == '=') return Token(Token::AndAndEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::AndAnd, getCurrentLocation());
            }
            if (ch == '=') return Token(Token::AndEqual, getCurrentLocation());
            unreadChar(ch);
            return Token(Token::And, getCurrentLocation());
        case '|':
            ch = readChar();
            if (ch == '|') {
                ch = readChar();
                if (ch == '=') return Token(Token::OrOrEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::OrOr, getCurrentLocation());
            }
            if (ch == '=') return Token(Token::OrEqual, getCurrentLocation());
            unreadChar(ch);
            return Token(Token::Or, getCurrentLocation());
        case '^':
            ch = readChar();
            if (ch == '=') return Token(Token::XorEqual, getCurrentLocation());
            unreadChar(ch);
            return Token(Token::Xor, getCurrentLocation());
        case '~':
            return Token(Token::Tilde, getCurrentLocation());
        case '(':
            return Token(Token::LeftParen, getCurrentLocation());
        case ')':
            return Token(Token::RightParen, getCurrentLocation());
        case '[':
            return Token(Token::LeftBracket, getCurrentLocation());
        case ']':
            return Token(Token::RightBracket, getCurrentLocation());
        case '{':
            return Token(Token::LeftBrace, getCurrentLocation());
        case '}':
            return Token(Token::RightBrace, getCurrentLocation());
        case '.':
            ch = readChar();
            if (ch == '.') {
                char ch = readChar();
                if (ch == '.') return Token(Token::DotDotDot, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::DotDot, getCurrentLocation());
            }
            unreadChar(ch);
            return Token(Token::Dot, getCurrentLocation());
        case ',':
            return Token(Token::Comma, getCurrentLocation());
        case ';':
            return Token(Token::Semicolon, getCurrentLocation());
        case ':':
            return Token(Token::Colon, getCurrentLocation());
        case '?':
            ch = readChar();
            if (ch == '?') return Token(Token::QuestionQuestion, getCurrentLocation());
            unreadChar(ch);
            return Token(Token::QuestionMark, getCurrentLocation());
        case '\0':
            goto end;
        case '"':
            return readQuotedLiteral('"', Token::StringLiteral);
        case '\'':
            return readQuotedLiteral('\'', Token::CharacterLiteral);
        default:
            if (std::isdigit(ch)) return readNumber();

            if (!std::isalpha(ch) && ch != '_' && ch != '#') {
                REPORT_ERROR(firstLocation, "unknown token '" << (char)ch << "'");
            }

            const char* begin = currentFilePosition;
            const char* end = begin;
            do {
                end++;
            } while (std::isalnum(ch = readChar()) || ch == '_');
            unreadChar(ch);

            llvm::StringRef string(begin, end - begin);

            auto it = keywords.find(string);
            if (it != keywords.end()) {
                return Token(it->second, getCurrentLocation(), string);
            }

            if (string.starts_with("__")) {
                ERROR(getCurrentLocation(), "'__'-prefixed identifiers are reserved for the compiler");
            }

            return Token(Token::Identifier, getCurrentLocation(), string);
        }
    }

end:
    return Token(Token::None, getCurrentLocation());
}
