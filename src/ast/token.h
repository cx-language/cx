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
        Const,
        Continue,
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
        Private,
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
        HashIf,
        HashElse,
        HashEndif,
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

    Token(Token::Kind kind, SourceLocation location, llvm::StringRef string = {});
    Token::Kind getKind() const { return kind; }
    operator Token::Kind() const { return kind; }
    llvm::StringRef getString() const { return string; }
    SourceLocation getLocation() const { return location; }
    bool is(Token::Kind kind) const { return this->kind == kind; }
    template<typename... T>
    bool is(Token::Kind kind, T... kinds) const {
        return is(kind) || is(kinds...);
    }
    int64_t getIntegerValue() const;
    long double getFloatingPointValue() const;

private:
    Token::Kind kind;
    llvm::StringRef string; ///< The substring in the source code representing this token.
    SourceLocation location;
};

struct UnaryOperator {
    UnaryOperator(Token token);
    Token::Kind getKind() const { return kind; }
    operator Token::Kind() const { return kind; }

private:
    Token::Kind kind;
};

struct BinaryOperator {
    BinaryOperator(Token token);
    Token::Kind getKind() const { return kind; }
    operator Token::Kind() const { return kind; }

private:
    Token::Kind kind;
};

bool isBinaryOperator(Token::Kind tokenKind);
bool isUnaryOperator(Token::Kind tokenKind);
bool isAssignmentOperator(Token::Kind tokenKind);
bool isCompoundAssignmentOperator(Token::Kind tokenKind);
bool isComparisonOperator(Token::Kind tokenKind);
bool isBitwiseOperator(Token::Kind tokenKind);
bool isOverloadable(Token::Kind tokenKind);
int getPrecedence(Token::Kind tokenKind);
std::string getFunctionName(Token::Kind tokenKind);

/// Strips the trailing '=' from a compound assignment operator.
/// E.g. given '+=', returns '+', and so on.
inline Token::Kind withoutCompoundEqSuffix(Token::Kind tokenKind) {
    return static_cast<Token::Kind>(static_cast<int>(tokenKind) - 1);
}

const char* toString(Token::Kind tokenKind);
std::ostream& operator<<(std::ostream& stream, Token::Kind tokenKind);

} // namespace delta
