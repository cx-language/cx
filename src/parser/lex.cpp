#include "lex.h"
#include <cctype>
#include <vector>
#include <string>
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

std::vector<std::unique_ptr<llvm::MemoryBuffer>> Lexer::fileBuffers;

Lexer::Lexer(std::unique_ptr<llvm::MemoryBuffer> input)
: firstLocation(input->getBufferIdentifier().data(), 1, 0),
  lastLocation(input->getBufferIdentifier().data(), 1, 0) {
    fileBuffers.emplace_back(std::move(input));
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
            error(startLocation, "unterminated block comment");
        }
    }
}

Token Lexer::readQuotedLiteral(char delimiter, Token::Kind literalKind) {
    const char* begin = currentFilePosition;
    const char* end = begin + 2;
    int ch;

    while ((ch = readChar()) != delimiter || *(end - 2) == '\\') {
        if (ch == '\n') {
            SourceLocation newlineLocation = firstLocation;
            newlineLocation.column += end - begin - 1;
            error(newlineLocation, "newline inside ", toString(literalKind));
        }
        end++;
    }

    return Token(literalKind, getCurrentLocation(), llvm::StringRef(begin, end - begin));
}

Token Lexer::readNumber() {
    const char* const begin = currentFilePosition;
    const char* end = begin + 1;
    bool isFloat = false;
    char ch;

    switch (ch = readChar()) {
        case 'b':
            if (begin[0] != '0') goto end;
            end++;
            while (true) {
                switch (ch = readChar()) {
                    case '0': case '1':
                        end++;
                        break;
                    default:
                        if (std::isalnum(ch))
                            error(lastLocation, "invalid digit '", ch, "' in binary literal");
                        if (end == begin + 2)
                            error(firstLocation, "binary literal must have at least one digit after '0b'");
                        goto end;
                }
            }
            break;
        case 'o':
            if (begin[0] != '0') goto end;
            end++;
            while (true) {
                switch (ch = readChar()) {
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7':
                        end++;
                        break;
                    default:
                        if (std::isalnum(ch))
                            error(lastLocation, "invalid digit '", ch, "' in octal literal");
                        if (end == begin + 2)
                            error(firstLocation, "octal literal must have at least one digit after '0o'");
                        goto end;
                }
            }
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (begin[0] == '0')
                error(firstLocation, "numbers cannot start with 0[0-9], use 0o prefix for octal literal");
            LLVM_FALLTHROUGH;
        default:
            while (true) {
                switch (ch) {
                    case '.':
                        if (isFloat) goto end;
                        isFloat = true;
                        LLVM_FALLTHROUGH;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        end++;
                        break;
                    default:
                        goto end;
                }
                ch = readChar();
            }
            break;
        case 'x':
            if (begin[0] != '0') goto end;
            end++;
            int lettercase = 0; // 0 -> not set yet, >0 -> uppercase, <0 -> lowercase
            while (true) {
                switch (ch = readChar()) {
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        end++;
                        break;
                    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                        if (lettercase < 0) error(lastLocation, "mixed letter case in hex literal");
                        end++;
                        lettercase = 1;
                        break;
                    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                        if (lettercase > 0) error(lastLocation, "mixed letter case in hex literal");
                        end++;
                        lettercase = -1;
                        break;
                    default:
                        if (std::isalnum(ch))
                            error(lastLocation, "invalid digit '", ch, "' in hex literal");
                        if (end == begin + 2)
                            error(firstLocation, "hex literal must have at least one digit after '0x'");
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

    return Token(isFloat ? Token::FloatLiteral : Token::IntegerLiteral, getCurrentLocation(),
                 llvm::StringRef(begin, end - begin));
}

static const llvm::StringMap<Token::Kind> keywords = {
    {"addressof",     Token::Addressof},
    {"break",         Token::Break},
    {"case",          Token::Case},
    {"cast",          Token::Cast},
    {"class",         Token::Class},
    {"const",         Token::Const},
    {"continue",      Token::Continue},
    {"default",       Token::Default},
    {"defer",         Token::Defer},
    {"deinit",        Token::Deinit},
    {"else",          Token::Else},
    {"enum",          Token::Enum},
    {"extern",        Token::Extern},
    {"false",         Token::False},
    {"for",           Token::For},
    {"func",          Token::Func},
    {"if",            Token::If},
    {"import",        Token::Import},
    {"in",            Token::In},
    {"init",          Token::Init},
    {"interface",     Token::Interface},
    {"let",           Token::Let},
    {"mutable",       Token::Mutable},
    {"mutating",      Token::Mutating},
    {"null",          Token::Null},
    {"return",        Token::Return},
    {"sizeof",        Token::Sizeof},
    {"struct",        Token::Struct},
    {"switch",        Token::Switch},
    {"this",          Token::This},
    {"true",          Token::True},
    {"undefined",     Token::Undefined},
    {"var",           Token::Var},
    {"while",         Token::While},
    {"_",             Token::Underscore},
    {"#if",           Token::HashIf},
    {"#else",         Token::HashElse},
    {"#endif",        Token::HashEndif},
};

Token Lexer::nextToken() {
    while (true) {
        char ch = readChar();
        firstLocation.line = lastLocation.line;
        firstLocation.column = lastLocation.column;

        switch (ch) {
            case ' ': case '\t': case '\r': case '\n':
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
            case '~': return Token(Token::Tilde, getCurrentLocation());
            case '(': return Token(Token::LeftParen, getCurrentLocation());
            case ')': return Token(Token::RightParen, getCurrentLocation());
            case '[': return Token(Token::LeftBracket, getCurrentLocation());
            case ']': return Token(Token::RightBracket, getCurrentLocation());
            case '{': return Token(Token::LeftBrace, getCurrentLocation());
            case '}': return Token(Token::RightBrace, getCurrentLocation());
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
            case ',': return Token(Token::Comma, getCurrentLocation());
            case ';': return Token(Token::Semicolon, getCurrentLocation());
            case ':': return Token(Token::Colon, getCurrentLocation());
            case '?': return Token(Token::QuestionMark, getCurrentLocation());
            case '\0':
                goto end;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return readNumber();
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
            case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
            case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
            case 'V': case 'W': case 'X': case 'Y': case 'Z':
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
            case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
            case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
            case 'v': case 'w': case 'x': case 'y': case 'z': case '_': case '#': {
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
            case '"':
                return readQuotedLiteral('"', Token::StringLiteral);
            case '\'':
                return readQuotedLiteral('\'', Token::CharacterLiteral);
            default:
                error(firstLocation, "unknown token '", (char) ch, "'");
        }
    }

end:
    return Token(Token::None, getCurrentLocation());
}
