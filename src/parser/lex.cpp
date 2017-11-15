#include "lex.h"
#include <vector>
#include <string>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/MemoryBuffer.h>
#include "parse.h"
#include "../ast/token.h"
#include "../support/utility.h"

using namespace delta;

namespace {

std::vector<std::unique_ptr<llvm::MemoryBuffer>> fileBuffers;
const char* currentFilePosition;

}

namespace delta {

extern const char* currentFilePath;
extern SourceLocation firstLocation;
extern SourceLocation lastLocation;

void initLexer(std::unique_ptr<llvm::MemoryBuffer> input) {
    currentFilePath = input->getBufferIdentifier().data();
    fileBuffers.emplace_back(std::move(input));
    currentFilePosition = fileBuffers.back()->getBufferStart() - 1;

    firstLocation = SourceLocation(currentFilePath, 1, 0);
    lastLocation = SourceLocation(currentFilePath, 1, 0);
}

}

namespace {

inline char readChar() {
    char ch = *++currentFilePosition;
    if (ch != '\n') {
        lastLocation.column++;
    } else {
        lastLocation.line++;
        lastLocation.column = 0;
    }
    return ch;
}

inline void unreadChar(char ch) {
    if (ch != '\n') {
        lastLocation.column--;
    } else {
        lastLocation.line--;
        // lastLocation.column can be left as is because the next readChar() call will reset it anyways.
    }
    currentFilePosition--;
}

static void readBlockComment(SourceLocation startLocation) {
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

static Token readQuotedLiteral(char delimiter, TokenKind literalKind) {
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

    return Token(literalKind, llvm::StringRef(begin, end - begin));
}

inline Token readNumber() {
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
        unreadChar('.'); // Lex the '.' as a Token::DOT.
        isFloat = false;
        end--;
    }

    return Token(isFloat ? FLOAT_LITERAL : INT_LITERAL, llvm::StringRef(begin, end - begin));
}

const llvm::StringMap<TokenKind> keywords = {
    {"break",         BREAK},
    {"case",          CASE},
    {"cast",          CAST},
    {"class",         CLASS},
    {"const",         CONST},
    {"default",       DEFAULT},
    {"defer",         DEFER},
    {"deinit",        DEINIT},
    {"else",          ELSE},
    {"enum",          ENUM},
    {"extern",        EXTERN},
    {"false",         FALSE},
    {"for",           FOR},
    {"func",          FUNC},
    {"if",            IF},
    {"import",        IMPORT},
    {"in",            IN},
    {"init",          INIT},
    {"interface",     INTERFACE},
    {"let",           LET},
    {"mutable",       MUTABLE},
    {"mutating",      MUTATING},
    {"null",          NULL_LITERAL},
    {"return",        RETURN},
    {"sizeof",        SIZEOF},
    {"struct",        STRUCT},
    {"switch",        SWITCH},
    {"this",          THIS},
    {"true",          TRUE},
    {"undefined",     UNDEFINED},
    {"var",           VAR},
    {"while",         WHILE},
    {"_",             UNDERSCORE},
};

} // anonymous namespace

Token delta::lex() {
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
                    return SLASH_EQ;
                } else {
                    unreadChar(ch);
                    return SLASH;
                }
                break;
            case '+':
                ch = readChar();
                if (ch == '+') return INCREMENT;
                if (ch == '=') return PLUS_EQ;
                unreadChar(ch);
                return PLUS;
            case '-':
                ch = readChar();
                if (ch == '-') return DECREMENT;
                if (ch == '>') return RARROW;
                if (ch == '=') return MINUS_EQ;
                unreadChar(ch);
                return MINUS;
            case '*':
                ch = readChar();
                if (ch == '=') return STAR_EQ;
                unreadChar(ch);
                return STAR;
            case '%':
                ch = readChar();
                if (ch == '=') return MOD_EQ;
                unreadChar(ch);
                return MOD;
            case '<':
                ch = readChar();
                if (ch == '=') return LE;
                if (ch == '<') {
                    ch = readChar();
                    if (ch == '=') return LSHIFT_EQ;
                    unreadChar(ch);
                    return LSHIFT;
                }
                unreadChar(ch);
                return LT;
            case '>':
                ch = readChar();
                if (ch == '=') return GE;
                if (ch == '>') {
                    ch = readChar();
                    if (ch == '=') return RSHIFT_EQ;
                    unreadChar(ch);
                    return RSHIFT;
                }
                unreadChar(ch);
                return GT;
            case '=':
                ch = readChar();
                if (ch == '=') return EQ;
                unreadChar(ch);
                return ASSIGN;
            case '!':
                ch = readChar();
                if (ch == '=') return NE;
                unreadChar(ch);
                return NOT;
            case '&':
                ch = readChar();
                if (ch == '&') {
                    ch = readChar();
                    if (ch == '=') return AND_AND_EQ;
                    unreadChar(ch);
                    return AND_AND;
                }
                if (ch == '=') return AND_EQ;
                unreadChar(ch);
                return AND;
            case '|':
                ch = readChar();
                if (ch == '|') {
                    ch = readChar();
                    if (ch == '=') return OR_OR_EQ;
                    unreadChar(ch);
                    return OR_OR;
                }
                if (ch == '=') return OR_EQ;
                unreadChar(ch);
                return OR;
            case '^':
                ch = readChar();
                if (ch == '=') return XOR_EQ;
                unreadChar(ch);
                return XOR;
            case '~': return COMPL;
            case '(': return LPAREN;
            case ')': return RPAREN;
            case '[': return LBRACKET;
            case ']': return RBRACKET;
            case '{': return LBRACE;
            case '}': return RBRACE;
            case '.':
                ch = readChar();
                if (ch == '.') {
                    char ch = readChar();
                    if (ch == '.') return DOTDOTDOT;
                    unreadChar(ch);
                    return DOTDOT;
                }
                unreadChar(ch);
                return DOT;
            case ',': return COMMA;
            case ';': return SEMICOLON;
            case ':': return COLON;
            case '?': return QUESTION_MARK;
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
            case 'v': case 'w': case 'x': case 'y': case 'z': case '_': {
                const char* begin = currentFilePosition;
                const char* end = begin;
                do {
                    end++;
                } while (std::isalnum(ch = readChar()) || ch == '_');
                unreadChar(ch);

                llvm::StringRef string(begin, end - begin);

                auto it = keywords.find(string);
                if (it != keywords.end()) {
                    return Token(it->second, string);
                }

                return Token(IDENTIFIER, string);
            }
            case '"':
                return readQuotedLiteral('"', STRING_LITERAL);
            case '\'':
                return readQuotedLiteral('\'', CHARACTER_LITERAL);
            default:
                error(firstLocation, "unknown token '", (char) ch, "'");
        }
    }

end:
    return NO_TOKEN;
}
