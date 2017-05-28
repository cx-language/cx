#include "lex.h"
#include <vector>
#include <string>
#include <cstdio>
#include <unordered_map>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include "parse.h"
#include "../ast/token.h"
#include "../driver/utility.h"

using namespace delta;

namespace {

std::vector<std::string> fileBuffers;
const char* currentFilePosition;

}

namespace delta {

extern SrcLoc firstLoc;
extern SrcLoc lastLoc;

void initLexer(const char* filePath) {
    std::FILE* inputFile = std::fopen(filePath, "rb");
    if (!inputFile) {
        printErrorAndExit("no such file: '", filePath, "'");
    }

    std::string contents;
    std::fseek(inputFile, 0, SEEK_END);
    contents.resize(std::ftell(inputFile));
    std::rewind(inputFile);
    std::fread(&contents[0], 1, contents.size(), inputFile);
    std::fclose(inputFile);
    fileBuffers.emplace_back(std::move(contents));
    currentFilePosition = fileBuffers.back().data() - 1;

    firstLoc = SrcLoc(filePath, 1, 0);
    lastLoc = SrcLoc(filePath, 1, 0);
}

}

namespace {

inline char readChar() {
    char ch = *++currentFilePosition;
    if (ch != '\n') {
        lastLoc.column++;
    } else {
        lastLoc.line++;
        lastLoc.column = 0;
    }
    return ch;
}

inline void unreadChar(char ch) {
    if (ch != '\n') {
        lastLoc.column--;
    } else {
        lastLoc.line--;
        // lastLoc.column can be left as is because the next readChar() call will reset it anyways.
    }
    currentFilePosition--;
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
                            error(lastLoc, "invalid digit '", ch, "' in binary literal");
                        if (end == begin + 2)
                            error(firstLoc, "binary literal must have at least one digit after '0b'");
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
                            error(lastLoc, "invalid digit '", ch, "' in octal literal");
                        if (end == begin + 2)
                            error(firstLoc, "octal literal must have at least one digit after '0o'");
                        goto end;
                }
            }
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (begin[0] == '0')
                error(firstLoc, "numbers cannot start with 0[0-9], use 0o prefix for octal literal");
            // fallthrough
        default:
            while (true) {
                switch (ch) {
                    case '.':
                        if (isFloat) goto end;
                        isFloat = true;
                        // fallthrough
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
                        if (lettercase < 0) error(lastLoc, "mixed letter case in hex literal");
                        end++;
                        lettercase = 1;
                        break;
                    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                        if (lettercase > 0) error(lastLoc, "mixed letter case in hex literal");
                        end++;
                        lettercase = -1;
                        break;
                    default:
                        if (std::isalnum(ch))
                            error(lastLoc, "invalid digit '", ch, "' in hex literal");
                        if (end == begin + 2)
                            error(firstLoc, "hex literal must have at least one digit after '0x'");
                        goto end;
                }
            }
            break;
    }

end:
    unreadChar(ch);

    assert(begin != end);
    if (end[-1] == '.') {
        unreadChar('.'); // Lex the '.' as a Token::DOT.
        isFloat = false;
        end--;
    }

    return Token(isFloat ? FLOAT_LITERAL : INT_LITERAL, llvm::StringRef(begin, end - begin));
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
    {"for",           FOR},
    {"func",          FUNC},
    {"if",            IF},
    {"import",        IMPORT},
    {"in",            IN},
    {"init",          INIT},
    {"interface",     INTERFACE},
    {"mutable",       MUTABLE},
    {"mutating",      MUTATING},
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
    while (true) {
        char ch = readChar();
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
            case '~':
                return COMPL;
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
                }
                unreadChar(ch);
                return DOT;
            case ',': return COMMA;
            case ';': return SEMICOLON;
            case ':':
                ch = readChar();
                if (ch == ':') return COLON_COLON;
                unreadChar(ch);
                return COLON;
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

                auto it = keywords.find(llvm::StringRef(begin, end - begin));
                if (it != keywords.end()) return it->second;

                return Token(IDENTIFIER, llvm::StringRef(begin, end - begin));
            }
            case '"': {
                const char* begin = currentFilePosition;
                const char* end = begin + 2;
                while ((ch = readChar()) != '"') {
                    if (ch == '\n') error(firstLoc, "newline inside string literal");
                    end++;
                }
                return Token(STRING_LITERAL, llvm::StringRef(begin, end - begin));
            }
            default:
                error(firstLoc, "unknown token '", (char) ch, "'");
        }
    }

end:
    return NO_TOKEN;
}
