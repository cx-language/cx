#pragma once

#include <ostream>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)
#include "../ast/location.h"

namespace delta {

enum TokenKind {
    NO_TOKEN,
    NEWLINE, // Not lexed.
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHARACTER_LITERAL,
    BREAK, // Keywords...
    CASE,
    CAST,
    CLASS,
    CONST,
    DEFAULT,
    DEFER,
    DEINIT,
    ELSE,
    ENUM,
    EXTERN,
    FALSE,
    FOR,
    FUNC,
    IF,
    IMPORT,
    IN,
    INIT,
    INTERFACE,
    LET,
    MUTABLE,
    MUTATING,
    NULL_LITERAL,
    RETURN,
    SIZEOF,
    STRUCT,
    SWITCH,
    THIS,
    TRUE,
    UNDEFINED,
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
    MOD,
    MOD_EQ,
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
    DOTDOT,
    DOTDOTDOT,
    COMMA,
    COLON,
    SEMICOLON,
    RARROW,
    QUESTION_MARK,
    TOKEN_COUNT
};

struct Token {
    Token(TokenKind kind, llvm::StringRef string = {});
    TokenKind getKind() const { return kind; }
    operator TokenKind() const { return kind; }
    llvm::StringRef getString() const { return string; }
    SourceLocation getLocation() const { return location; }
    bool is(TokenKind kind) const { return this->kind == kind; }
    template<typename... T>
    bool is(TokenKind kind, T... kinds) const { return is(kind) || is(kinds...); }
    bool isBinaryOperator() const;
    bool isPrefixOperator() const;
    bool isAssignmentOperator() const;
    bool isCompoundAssignmentOperator() const;
    bool isOverloadable() const;
    int getPrecedence() const;
    int64_t getIntegerValue() const;
    long double getFloatingPointValue() const;

    /// Strips the trailing '=' from a compound assignment operator.
    /// E.g. given '+=', returns '+', and so on.
    Token withoutCompoundEqSuffix() const { return Token(TokenKind(kind - 1)); }

private:
    TokenKind kind;
    llvm::StringRef string; ///< The substring in the source code representing this token.
    SourceLocation location;
};

struct PrefixOperator {
    PrefixOperator(Token token);
    TokenKind getKind() const { return kind; }
    operator TokenKind() const { return kind; }

private:
    TokenKind kind;
};

struct BinaryOperator {
    BinaryOperator(Token token);
    TokenKind getKind() const { return kind; }
    operator TokenKind() const { return kind; }
    bool isComparisonOperator() const;
    bool isBitwiseOperator() const;
    std::string getFunctionName() const;

private:
    TokenKind kind;
};

const char* toString(TokenKind tokenKind);
std::ostream& operator<<(std::ostream& stream, TokenKind tokenKind);

}
