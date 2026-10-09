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
    // Skip a UTF-8 BOM so files saved by Windows editors lex cleanly.
    if (buffer.getBuffer().starts_with("\xEF\xBB\xBF")) {
        currentFilePosition += 3;
    }
}

const char* Lexer::getFilePath() const {
    return buffer.getBufferIdentifier().data();
}

Location Lexer::getCurrentLocation() const {
    return Location(getFilePath(), firstLocation.line, firstLocation.column);
}

char Lexer::readChar() {
    // EOF is sticky: never read past the end of the buffer.
    if (currentFilePosition >= buffer.getBufferEnd()) {
        return '\0';
    }
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
        } else if (ch == '"') {
            // Skip string literals so comment markers inside strings don't
            // affect nesting. Mirrors string lexing: backslash escapes the
            // next char, while a quote unterminated on its line is an
            // ordinary char instead of starting a string.
            const char* p = currentFilePosition + 1;
            while (p < buffer.getBufferEnd()) {
                char c = *p;
                if (c == '\\') {
                    if (p + 1 >= buffer.getBufferEnd()) break;
                    p += 2;
                } else if (c == '"' || c == '\0' || c == '\n' || c == '\r') {
                    break;
                } else {
                    ++p;
                }
            }
            if (p < buffer.getBufferEnd() && *p == '"') {
                while (currentFilePosition < p)
                    readChar();
            }
        } else if (ch == '\0') {
            unreadChar(ch);
            REPORT_ERROR_RANGE(startLocation, getIdentifierEndLocation(startLocation, "/*"), "unterminated block comment");
            break;
        }
    }
}

Token Lexer::nextToken() {
    firstLocation.line = lastLocation.line;
    firstLocation.column = lastLocation.column;

    if (pendingInterpStart) {
        pendingInterpStart = false;
        readChar();
        LexFrame frame;
        frame.isCode = true;
        frameStack.push_back(frame);
        return Token(Token::InterpStart, getCurrentLocation(), "{");
    }

    if (!frameStack.empty() && frameStack.back().isCode) return lexCodeToken();
    if (!frameStack.empty()) return lexStringResume();
    return lexToken();
}

Token Lexer::lexCodeToken() {
    size_t frameIndex = frameStack.size() - 1;
    Token token = lexToken();

    if (token.kind == Token::None) {
        ERROR(getCurrentLocation(), "unterminated interpolation, expected '}'");
    }

    // Braces inside nested strings never surface as brace tokens, so only code
    // braces reach the depth count, and trivia before `}` needs no peeking.
    if (token.kind == Token::LeftBrace) {
        frameStack[frameIndex].codeFrame.braceDepth++;
    } else if (token.kind == Token::RightBrace) {
        if (frameStack[frameIndex].codeFrame.braceDepth == 0) {
            frameStack.pop_back();
            frameStack.back().stringFrame.contentBegin = currentFilePosition + 1;
            return Token(Token::InterpEnd, token.location, "}");
        }
        frameStack[frameIndex].codeFrame.braceDepth--;
    }
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

        if (stringFrame.delimiter == '"' && ch == '{') {
            // Interpolation trigger: unread back to '{' so InterpStart
            // consumption stays uniform, then suspend with a flag.
            const char* bracePos = currentFilePosition;
            unreadChar('{');
            pendingInterpStart = true;

            if (bracePos > stringFrame.contentBegin) {
                return Token(Token::StringLiteral, getCurrentLocation(), llvm::StringRef(stringFrame.contentBegin, bracePos - stringFrame.contentBegin));
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
        } else if (delimiter == '"' && ch == '{') {
            // Interpolation trigger: suspend with a flag and return the
            // chunk before '{'. Chunks exclude quotes; the parser builds
            // them without quote-stripping (unlike plain literals below).
            const char* bracePos = currentFilePosition;
            unreadChar('{');
            pendingInterpStart = true;

            LexFrame frame;
            frame.isCode = false;
            frame.stringFrame.delimiter = delimiter;
            frame.stringFrame.quotePos = begin;
            frameStack.push_back(frame);

            if (bracePos > begin + 1) {
                return Token(literalKind, getCurrentLocation(), llvm::StringRef(begin + 1, bracePos - (begin + 1)));
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
    bool sawOverflow = false;
    bool sawSeparator = false;
    bool sawNonSeparator = false;
    bool sawExponent = false;
    uint64_t intValue = *begin - '0';
    char ch = readChar();

    auto appendDigit = [&](uint64_t digit, uint64_t base) {
        // Deferred to the end of the literal so the range covers all digits.
        if (intValue > (std::numeric_limits<uint64_t>::max() - digit) / base) sawOverflow = true;
        intValue = intValue * base + digit;
    };

    // Binary and octal share the scan: digits, '_' separators, then a non-digit terminator.
    auto readRadixLiteral = [&](auto isDigit, uint64_t base, const char* kindName, char prefix) {
        end++;
        while (true) {
            ch = readChar();
            if (isDigit(ch)) {
                appendDigit(uint64_t(ch - '0'), base);
                sawNonSeparator = true;
                end++;
                continue;
            }
            if (ch == '_') {
                end++;
                continue;
            }
            if (std::isalnum(ch)) ERROR(lastLocation, "invalid digit '" << ch << "' in " << kindName << " literal");
            if (end == begin + 2 || !sawNonSeparator)
                ERROR_RANGE(firstLocation, getIdentifierEndLocation(firstLocation, {begin, size_t(end - begin)}),
                            kindName << " literal must have at least one digit after '0" << prefix << "'");
            return;
        }
    };

    switch (ch) {
    case 'b':
        if (begin[0] != '0') goto end;
        readRadixLiteral([](char digit) { return digit == '0' || digit == '1'; }, 2, "binary", 'b');
        goto end;
    case 'o':
        if (begin[0] != '0') goto end;
        readRadixLiteral([](char digit) { return digit >= '0' && digit <= '7'; }, 8, "octal", 'o');
        goto end;
    default:
        if (std::isdigit(ch) && begin[0] == '0') {
            ERROR_RANGE(firstLocation, lastLocation.nextColumn(), "numbers cannot start with 0[0-9], use 0o prefix for octal literal");
        }

        while (true) {
            if (ch == '.' && !isFloat) {
                if (sawSeparator) ERROR_RANGE(firstLocation, lastLocation.nextColumn(), "float literals cannot contain separators");
                isFloat = true;
            } else if ((ch == 'e' || ch == 'E') && !sawExponent) {
                if (sawSeparator) ERROR_RANGE(firstLocation, lastLocation.nextColumn(), "float literals cannot contain separators");
                end++;
                ch = readChar();
                if (ch == '+' || ch == '-') {
                    end++;
                    ch = readChar();
                }
                if (!std::isdigit(ch)) ERROR_RANGE(firstLocation, lastLocation.nextColumn(), "float literal exponent must have at least one digit");
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
                if (isFloat) ERROR_RANGE(firstLocation, lastLocation.nextColumn(), "float literals cannot contain separators");
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
                if (end == begin + 2 || !sawNonSeparator)
                    ERROR_RANGE(firstLocation, getIdentifierEndLocation(firstLocation, {begin, size_t(end - begin)}),
                                "hex literal must have at least one digit after '0x'");
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

    if (sawOverflow) {
        ERROR_RANGE(firstLocation, getIdentifierEndLocation(firstLocation, {begin, size_t(end - begin)}), "integer literal is too large");
    }

    if (isFloat) return Token(Token::FloatLiteral, getCurrentLocation(), llvm::StringRef(begin, end - begin));
    return Token(getCurrentLocation(), intValue, int(end - begin));
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
    {"implicit", Token::Implicit},
    {"import", Token::Import},
    {"in", Token::In},
    {"interface", Token::Interface},
    {"is", Token::Is},
    {"namespace", Token::Namespace},
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
    {"union", Token::Union},
    {"using", Token::Using},
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
            if (ch == '%') {
                ch = readChar();
                if (ch == '=') return Token(Token::PlusWrapEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::PlusWrap, getCurrentLocation());
            }
            if (ch == '|') {
                ch = readChar();
                if (ch == '=') return Token(Token::PlusSatEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::PlusSat, getCurrentLocation());
            }
            unreadChar(ch);
            return Token(Token::Plus, getCurrentLocation());
        case '-':
            ch = readChar();
            if (ch == '-') return Token(Token::Decrement, getCurrentLocation());
            if (ch == '=') return Token(Token::MinusEqual, getCurrentLocation());
            if (ch == '%') {
                ch = readChar();
                if (ch == '=') return Token(Token::MinusWrapEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::MinusWrap, getCurrentLocation());
            }
            if (ch == '|') {
                ch = readChar();
                if (ch == '=') return Token(Token::MinusSatEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::MinusSat, getCurrentLocation());
            }
            if (ch == '>') {
                // '->' was the lambda arrow before it was changed to '=>', and C
                // programmers reach for it for member access; recover as a fat
                // arrow so parsing continues and only this error is reported.
                REPORT_ERROR_RANGE(getCurrentLocation(), getIdentifierEndLocation(getCurrentLocation(), "->"),
                                   "unexpected '->', use '=>' for lambdas or '.' for member access");
                return Token(Token::FatArrow, getCurrentLocation());
            }
            unreadChar(ch);
            return Token(Token::Minus, getCurrentLocation());
        case '*':
            ch = readChar();
            if (ch == '=') return Token(Token::StarEqual, getCurrentLocation());
            if (ch == '%') {
                ch = readChar();
                if (ch == '=') return Token(Token::StarWrapEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::StarWrap, getCurrentLocation());
            }
            if (ch == '|') {
                ch = readChar();
                if (ch == '=') return Token(Token::StarSatEqual, getCurrentLocation());
                unreadChar(ch);
                return Token(Token::StarSat, getCurrentLocation());
            }
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
                if (ch == '|') {
                    ch = readChar();
                    if (ch == '=') return Token(Token::LeftShiftSatEqual, getCurrentLocation());
                    unreadChar(ch);
                    return Token(Token::LeftShiftSat, getCurrentLocation());
                }
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
        case '@':
            return Token(Token::At, getCurrentLocation());
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
                ERROR_RANGE(getCurrentLocation(), getIdentifierEndLocation(getCurrentLocation(), string),
                            "'__'-prefixed identifiers are reserved for the compiler");
            }

            return Token(Token::Identifier, getCurrentLocation(), string);
        }
    }

end:
    return Token(Token::None, getCurrentLocation());
}
