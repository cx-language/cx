#pragma once

#include <ostream>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)
#include "../ast/location.h"

namespace delta {

struct Token {
    enum Kind {
        None,
        Newline,
        Identifier,
        IntegerLiteral,
        FloatLiteral,
        StringLiteral,
        CharacterLiteral,
        Addressof,
        Break,
        Case,
        Cast,
        Class,
        Const,
        Default,
        Defer,
        Deinit,
        Else,
        Enum,
        Extern,
        False,
        For,
        Func,
        If,
        Import,
        In,
        Init,
        Interface,
        Let,
        Mutable,
        Mutating,
        Null,
        Return,
        Sizeof,
        Struct,
        Switch,
        This,
        True,
        Undefined,
        Var,
        While,
        Underscore,
        Equal,
        NotEqual,
        PointerEqual,
        PointerNotEqual,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        Plus,
        PlusEqual,
        Minus,
        MinusEqual,
        Star,
        StarEqual,
        Slash,
        SlashEqual,
        Modulo,
        ModuloEqual,
        Increment,
        Decrement,
        Not,
        And,
        AndEqual,
        AndAnd,
        AndAndEqual,
        Or,
        OrEqual,
        OrOr,
        OrOrEqual,
        Xor,
        XorEqual,
        Tilde,
        LeftShift,
        LeftShiftEqual,
        RightShift,
        RightShiftEqual,
        Assignment,
        LeftParen,
        RightParen,
        LeftBracket,
        RightBracket,
        LeftBrace,
        RightBrace,
        Dot,
        DotDot,
        DotDotDot,
        Comma,
        Colon,
        Semicolon,
        RightArrow,
        QuestionMark,
        TokenCount
    };

    Token(Token::Kind kind, llvm::StringRef string = {});
    Token::Kind getKind() const { return kind; }
    operator Token::Kind() const { return kind; }
    llvm::StringRef getString() const { return string; }
    SourceLocation getLocation() const { return location; }
    bool is(Token::Kind kind) const { return this->kind == kind; }
    template<typename... T>
    bool is(Token::Kind kind, T... kinds) const { return is(kind) || is(kinds...); }
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
    Token withoutCompoundEqSuffix() const {
        return Token(static_cast<Token::Kind>(static_cast<int>(kind) - 1));
    }

private:
    Token::Kind kind;
    llvm::StringRef string; ///< The substring in the source code representing this token.
    SourceLocation location;
};

struct PrefixOperator {
    PrefixOperator(Token token);
    Token::Kind getKind() const { return kind; }
    operator Token::Kind() const { return kind; }

private:
    Token::Kind kind;
};

struct BinaryOperator {
    BinaryOperator(Token token);
    Token::Kind getKind() const { return kind; }
    operator Token::Kind() const { return kind; }
    bool isComparisonOperator() const;
    bool isBitwiseOperator() const;
    std::string getFunctionName() const;

private:
    Token::Kind kind;
};

const char* toString(Token::Kind tokenKind);
std::ostream& operator<<(std::ostream& stream, Token::Kind tokenKind);

}
