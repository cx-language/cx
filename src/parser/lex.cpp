#include "lex.h"
#include <cctype>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)
#include "parse.h"
#include "../ast/token.h"
#include "../support/utility.h"

using namespace delta;

std::vector<llvm::MemoryBuffer*> Lexer::fileBuffers;

Lexer::Lexer(llvm::MemoryBuffer* input)
: firstLocation(input->getBufferIdentifier().data(), 1, 0), lastLocation(input->getBufferIdentifier().data(), 1, 0) {
    fileBuffers.push_back(input);
    currentFilePosition = fileBuffers.back()->getBufferStart() - 1;
}

const char* Lexer::getFilePath() const {
    return fileBuffers.back()->getBufferIdentifier().data();
}

SourceLocation Lexer::getCurrentLocation() const {
    return SourceLocation(getFilePath(), firstLocation.line, firstLocation.column);
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

void Lexer::readBlockComment(SourceLocation startLocation) {
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

Token Lexer::readQuotedLiteral(char delimiter, Token::Kind literalKind) {
    const char* begin = currentFilePosition;
    const char* end = begin + 2;
    int ch;

    while ((ch = readChar()) != delimiter || *(end - 2) == '\\') {
        if (ch == '\n' || ch == '\r') {
            SourceLocation newlineLocation = firstLocation;
            newlineLocation.column += end - begin - 1;
            ERROR(newlineLocation, "newline inside " << toString(literalKind));
        }
        end++;
    }

    return Token(literalKind, getCurrentLocation(), llvm::StringRef(begin, end - begin));
}

Token Lexer::readNumber() {
    const char* const begin = currentFilePosition;
    const char* end = begin + 1;
    bool isFloat = false;
    char ch = readChar();

    switch (ch) {
        case 'b':
            if (begin[0] != '0') goto end;
            end++;
            while (true) {
                ch = readChar();
                if (ch >= '0' && ch <= '1') {
                    end++;
                    continue;
                }
                if (std::isalnum(ch)) ERROR(lastLocation, "invalid digit '" << ch << "' in binary literal");
                if (end == begin + 2) ERROR(firstLocation, "binary literal must have at least one digit after '0b'");
                goto end;
            }
            break;
        case 'o':
            if (begin[0] != '0') goto end;
            end++;
            while (true) {
                ch = readChar();
                if (ch >= '0' && ch <= '7') {
                    end++;
                    continue;
                }
                if (std::isalnum(ch)) ERROR(lastLocation, "invalid digit '" << ch << "' in octal literal");
                if (end == begin + 2) ERROR(firstLocation, "octal literal must have at least one digit after '0o'");
                goto end;
            }
            break;
        default:
            if (std::isdigit(ch) && begin[0] == '0') {
                ERROR(firstLocation, "numbers cannot start with 0[0-9], use 0o prefix for octal literal");
            }

            while (true) {
                if (ch == '.') {
                    if (isFloat) goto end;
                    isFloat = true;
                } else if (!std::isdigit(ch)) {
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
                    end++;
                } else if (ch >= 'a' && ch <= 'f') {
                    if (lettercase > 0) ERROR(lastLocation, "mixed letter case in hex literal");
                    end++;
                    lettercase = -1;
                } else if (ch >= 'A' && ch <= 'F') {
                    if (lettercase < 0) ERROR(lastLocation, "mixed letter case in hex literal");
                    end++;
                    lettercase = 1;
                } else {
                    if (std::isalnum(ch)) ERROR(lastLocation, "invalid digit '" << ch << "' in hex literal");
                    if (end == begin + 2) ERROR(firstLocation, "hex literal must have at least one digit after '0x'");
                    goto end;
                }
            }
            break;
    }

end:
    unreadChar(ch);

    ASSERT(begin != end);
    if (end[-1] == '.') {
        unreadChar('.'); // Lex the '.' as a Token::Dot.
        isFloat = false;
        end--;
    }

    return Token(isFloat ? Token::FloatLiteral : Token::IntegerLiteral, getCurrentLocation(), llvm::StringRef(begin, end - begin));
}

static const llvm::StringMap<Token::Kind> keywords = {
    { "addressof", Token::Addressof },
    { "as", Token::As },
    { "break", Token::Break },
    { "case", Token::Case },
    { "const", Token::Const },
    { "continue", Token::Continue },
    { "default", Token::Default },
    { "defer", Token::Defer },
    { "else", Token::Else },
    { "enum", Token::Enum },
    { "extern", Token::Extern },
    { "false", Token::False },
    { "for", Token::For },
    { "if", Token::If },
    { "import", Token::Import },
    { "in", Token::In },
    { "interface", Token::Interface },
    { "null", Token::Null },
    { "private", Token::Private },
    { "return", Token::Return },
    { "sizeof", Token::Sizeof },
    { "struct", Token::Struct },
    { "switch", Token::Switch },
    { "this", Token::This },
    { "true", Token::True },
    { "undefined", Token::Undefined },
    { "var", Token::Var },
    { "while", Token::While },
    { "#if", Token::HashIf },
    { "#else", Token::HashElse },
    { "#endif", Token::HashEndif },
};

Token Lexer::nextToken() {
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
                if (ch == '>') return Token(Token::RightArrow, getCurrentLocation());
                if (ch == '=') return Token(Token::MinusEqual, getCurrentLocation());
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
                    ch = readChar();
                    if (ch == '=') return Token(Token::PointerEqual, getCurrentLocation());
                    unreadChar(ch);
                    return Token(Token::Equal, getCurrentLocation());
                }
                unreadChar(ch);
                return Token(Token::Assignment, getCurrentLocation());
            case '!':
                ch = readChar();
                if (ch == '=') {
                    ch = readChar();
                    if (ch == '=') return Token(Token::PointerNotEqual, getCurrentLocation());
                    unreadChar(ch);
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
                    REPORT_ERROR(firstLocation, "unknown token '" << (char) ch << "'");
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

                return Token(Token::Identifier, getCurrentLocation(), string);
        }
    }

end:
    return Token(Token::None, getCurrentLocation());
}
