#include <cstdio>
#include <cassert>
#include <unordered_map>

FILE* inputFile;

static inline int readChar() {
    return getc(inputFile);
}

static inline void unreadChar(int ch) {
    ungetc(ch, inputFile);
}

template<typename... Args>
[[noreturn]] static void error(Args&&... args) {
    std::cout << "error: ";
    using expander = int[];
    (void)expander{0, (void(std::cout << std::forward<Args>(args)), 0)...};
    std::cout << '\n';
    exit(1);
}

static inline void readNumber(const int base, char ch = 0) {
    std::string string;
    if (ch) string += (char) ch;

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
                        if (lettercase < 0) error("mixed letter case in hex literal");
                        string += (char) ch;
                        lettercase = 1;
                        break;
                    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                        if (lettercase > 0) error("mixed letter case in hex literal");
                        string += (char) ch;
                        lettercase = -1;
                        break;
                    default:
                        goto end;
                }
            }
            break;
        }
        default:
            assert(false);
    }

end:
    unreadChar(ch);
    yylval.number = std::strtoll(string.c_str(), nullptr, base);
}

static const std::unordered_map<std::string, int> keywords = {
    {"cast",          CAST},
    {"class",         CLASS},
    {"const",         CONST},
    {"else",          ELSE},
    {"extern",        EXTERN},
    {"false",         FALSE},
    {"func",          FUNC},
    {"if",            IF},
    {"init",          INIT},
    {"mutable",       MUTABLE},
    {"return",        RETURN},
    {"struct",        STRUCT},
    {"this",          THIS},
    {"true",          TRUE},
    {"uninitialized", UNINITIALIZED},
    {"var",           VAR},
    {"while",         WHILE},
};

int lex() {
    while (true) {
        int ch = readChar();

        switch (ch) {
            case ' ': case '\t': case '\r': case '\n':
                break; // skip whitespace
            case '/':
                ch = readChar();
                if (ch == '/') {
                    // comment until end of line
                    while (readChar() != '\n') { }
                } else {
                    unreadChar(ch);
                    return SLASH;
                }
                break;
            case '+':
                ch = readChar();
                if (ch == '+') return INCREMENT;
                unreadChar(ch);
                return PLUS;
            case '-':
                ch = readChar();
                if (ch == '-') return DECREMENT;
                if (ch == '>') return RARROW;
                unreadChar(ch);
                return MINUS;
            case '*':
                return STAR;
            case '<':
                ch = readChar();
                if (ch == '=') return LE;
                if (ch == '<') return LSHIFT;
                unreadChar(ch);
                return LT;
            case '>':
                ch = readChar();
                if (ch == '=') return GE;
                if (ch == '>') return RSHIFT;
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
                if (ch == '&') return AND_AND;
                unreadChar(ch);
                return AND;
            case '|':
                ch = readChar();
                if (ch == '|') return OR_OR;
                unreadChar(ch);
                return OR;
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
                    case 'b': readNumber(2); return NUMBER;
                    case 'o': readNumber(8); return NUMBER;
                    case 'x': readNumber(16); return NUMBER;
                    default:
                        if (std::isdigit(ch)) {
                            error("numbers cannot start with 0[0-9], use 0o prefix for octal literal");
                        }
                        if (std::isalpha(ch) || ch == '_') {
                            error("unexpected '", (char) ch, "'");
                        }
                        unreadChar(ch);
                        yylval.number = 0;
                        return NUMBER;
                }
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':
                readNumber(10, ch);
                return NUMBER;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
            case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
            case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
            case 'V': case 'W': case 'X': case 'Y': case 'Z':
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
            case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
            case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
            case 'v': case 'w': case 'x': case 'y': case 'z': {
                std::string string;
                do {
                    string += (char) ch;
                } while (std::isalnum(ch = readChar()) || ch == '_');
                unreadChar(ch);

                auto it = keywords.find(string);
                if (it != keywords.end()) return it->second;

                yylval.string = strndup(string.data(), string.length());
                return IDENTIFIER;
            }
            case '"': {
                std::string string;
                while ((ch = readChar()) != '"') {
                    if (ch == '\n') error("newline inside string literal");
                    string += (char) ch;
                }
                yylval.string = strndup(string.data(), string.length());
                return STRING_LITERAL;
            }
            default:
                std::cout << "error: unknown token '" << (char) ch << "'\n";
                exit(1);
        }
    }

end:
    return 0;
}
