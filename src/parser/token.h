#pragma once

#include <ostream>
#include "../ast/srcloc.h"

namespace delta {

enum TokenKind {
    NO_TOKEN,
    IDENTIFIER,
    NUMBER,
    STRING_LITERAL,
    BREAK, // Keywords...
    CASE,
    CAST,
    CLASS,
    CONST,
    DEFAULT,
    DEFER,
    DEINIT,
    ELSE,
    EXTERN,
    FALSE,
    FUNC,
    IF,
    IMPORT,
    INIT,
    MUTABLE,
    NULL_LITERAL,
    RETURN,
    STRUCT,
    SWITCH,
    THIS,
    TRUE,
    UNINITIALIZED,
    VAR,
    WHILE,
    UNDERSCORE,
    EQ, // Operators...
    NE,
    LT,
    LE,
    GT,
    GE,
    PLUS,
    PLUS_EQ,
    MINUS,
    MINUS_EQ,
    STAR,
    STAR_EQ,
    SLASH,
    SLASH_EQ,
    INCREMENT,
    DECREMENT,
    NOT,
    AND,
    AND_EQ,
    AND_AND,
    AND_AND_EQ,
    OR,
    OR_EQ,
    OR_OR,
    OR_OR_EQ,
    XOR,
    XOR_EQ,
    COMPL,
    LSHIFT,
    LSHIFT_EQ,
    RSHIFT,
    RSHIFT_EQ,
    ASSIGN, // Miscellaneous...
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    DOT,
    COMMA,
    COLON,
    COLON_COLON,
    SEMICOLON,
    RARROW,
};

struct Token {
    const TokenKind kind;
    union {
        const long long number;
        const char* const string;
    };

    Token(TokenKind kind);
    explicit Token(long long number);
    explicit Token(TokenKind token, char* value);
    operator TokenKind() const { return kind; }
    SrcLoc getLoc() const { return srcLoc; }
    bool isBinaryOperator() const;
    bool isPrefixOperator() const;
    int getPrecedence() const;

    /// Strips the trailing '=' from a compound assignment operator.
    /// E.g. given '+=', returns '+', and so on.
    Token withoutCompoundEqSuffix() const { return Token(TokenKind(kind - 1)); }

private:
    const SrcLoc srcLoc;
};

struct PrefixOperator {
    PrefixOperator(Token token);
    operator TokenKind() const { return kind; }
    TokenKind kind;
};

struct BinaryOperator {
    BinaryOperator(Token token);
    operator TokenKind() const { return kind; }
    bool isComparisonOperator() const;
    TokenKind kind;
};

std::ostream& operator<<(std::ostream& stream, TokenKind tokenKind);

}
