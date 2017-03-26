#include "lex.h"
#include <cstdio>
#include <unordered_map>
#include <llvm/Support/ErrorHandling.h>
#include "parse.h"
#include "token.h"
#include "../driver/utility.h"

using namespace delta;

namespace {

FILE* inputFile;

}

namespace delta {

extern const char* currentFileName;
SrcLoc firstLoc(nullptr, 1, 0);
SrcLoc lastLoc(nullptr, 1, 0);

void initLexer(const char* filePath) {
    if (inputFile) fclose(inputFile);
    inputFile = fopen(filePath, "r");
    if (!inputFile) {
        printErrorAndExit("no such file: '", filePath, "'");
    }

    firstLoc = SrcLoc(filePath, 1, 0);
    lastLoc = SrcLoc(filePath, 1, 0);
}

}

namespace {

inline int readChar() {
    int ch = getc(inputFile);
    if (ch != '\n') {
        lastLoc.column++;
    } else {
        lastLoc.line++;
        lastLoc.column = 0;
    }
    return ch;
}

inline void unreadChar(int ch) {
    if (ch != '\n') {
        lastLoc.column--;
    } else {
        lastLoc.line--;
        // lastLoc.column can be left as is because the next readChar() call will reset it anyways.
    }
    ungetc(ch, inputFile);
}

inline Token readNumber(const int base, char ch = 0) {
    std::string string;
    if (ch) string += (char) ch;
    bool isFloat = ch == '.';

    switch (base) {
        case 2:
            while (true) {
                switch (ch = readChar()) {
                    case '0': case '1':
                        string += (char) ch;
                        break;
                    default:
                        goto end;
                }
            }
            break;
        case 8:
            while (true) {
                switch (ch = readChar()) {
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7':
                        string += (char) ch;
                        break;
                    default:
                        goto end;
                }
            }
            break;
        case 10:
            while (true) {
                switch (ch = readChar()) {
                    case '.':
                        if (isFloat) goto end;
                        isFloat = true;
                        // fallthrough
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        string += (char) ch;
                        break;
                    default:
                        goto end;
                }
            }
            break;
        case 16: {
            int lettercase = 0; // 0 -> not set yet, >0 -> uppercase, <0 -> lowercase
            while (true) {
                switch (ch = readChar()) {
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        string += (char) ch;
                        break;
                    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                        if (lettercase < 0) error(lastLoc, "mixed letter case in hex literal");
                        string += (char) ch;
                        lettercase = 1;
                        break;
                    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                        if (lettercase > 0) error(lastLoc, "mixed letter case in hex literal");
                        string += (char) ch;
                        lettercase = -1;
                        break;
                    default:
                        goto end;
                }
            }
            break;
        }
        default: llvm_unreachable("invalid integer base");
    }

end:
    unreadChar(ch);

    assert(!string.empty());
    if (string.back() == '.')
        unreadChar('.'); // Lex the '.' as a Token::DOT.

    if (isFloat) {
        assert(base == 10 && "float literals must be base-10");
        return Token(std::strtold(string.c_str(), nullptr));
    }
    return Token(std::strtoll(string.c_str(), nullptr, base));
}

const std::unordered_map<std::string, TokenKind> keywords = {
    {"break",         BREAK},
    {"case",          CASE},
    {"cast",          CAST},
    {"class",         CLASS},
    {"const",         CONST},
    {"default",       DEFAULT},
    {"defer",         DEFER},
    {"deinit",        DEINIT},
    {"else",          ELSE},
    {"extern",        EXTERN},
    {"false",         FALSE},
    {"func",          FUNC},
    {"if",            IF},
    {"import",        IMPORT},
    {"init",          INIT},
    {"mutable",       MUTABLE},
    {"null",          NULL_LITERAL},
    {"return",        RETURN},
    {"struct",        STRUCT},
    {"switch",        SWITCH},
    {"this",          THIS},
    {"true",          TRUE},
    {"uninitialized", UNINITIALIZED},
    {"var",           VAR},
    {"while",         WHILE},
    {"_",             UNDERSCORE},
};

} // anonymous namespace

Token delta::lex() {
    assert(inputFile && "lexer not initialized");

    while (true) {
        int ch = readChar();
        firstLoc.line = lastLoc.line;
        firstLoc.column = lastLoc.column;

        switch (ch) {
            case ' ': case '\t': case '\r': case '\n':
                break; // skip whitespace
            case '/':
                ch = readChar();
                if (ch == '/') {
                    // comment until end of line
                    while (readChar() != '\n') { }
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
            case '~':
                return COMPL;
            case '(': return LPAREN;
            case ')': return RPAREN;
            case '[': return LBRACKET;
            case ']': return RBRACKET;
            case '{': return LBRACE;
            case '}': return RBRACE;
            case '.': return DOT;
            case ',': return COMMA;
            case ';': return SEMICOLON;
            case ':':
                ch = readChar();
                if (ch == ':') return COLON_COLON;
                unreadChar(ch);
                return COLON;
            case EOF:
                goto end;
            case '0':
                ch = readChar();
                switch (ch) {
                    case 'b': return readNumber(2);
                    case 'o': return readNumber(8);
                    case 'x': return readNumber(16);
                    default:
                        if (std::isdigit(ch)) {
                            error(firstLoc, "numbers cannot start with 0[0-9], use 0o prefix for octal literal");
                        }
                        if (std::isalpha(ch) || ch == '_') {
                            error(lastLoc, "unexpected '", (char) ch, "'");
                        }
                        if (ch == '.') {
                            return readNumber(10, '.');
                        }
                        unreadChar(ch);
                        return Token((long long) 0);
                }
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':
                return readNumber(10, ch);
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
            case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
            case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
            case 'V': case 'W': case 'X': case 'Y': case 'Z':
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
            case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
            case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
            case 'v': case 'w': case 'x': case 'y': case 'z': case '_': {
                std::string string;
                do {
                    string += (char) ch;
                } while (std::isalnum(ch = readChar()) || ch == '_');
                unreadChar(ch);

                auto it = keywords.find(string);
                if (it != keywords.end()) return it->second;

                return Token(IDENTIFIER, strndup(string.data(), string.length()));
            }
            case '"': {
                std::string string;
                while ((ch = readChar()) != '"') {
                    if (ch == '\n') error(firstLoc, "newline inside string literal");
                    string += (char) ch;
                }
                return Token(STRING_LITERAL, strndup(string.data(), string.length()));
            }
            default:
                error(firstLoc, "unknown token '", (char) ch, "'");
        }
    }

end:
    return NO_TOKEN;
}
