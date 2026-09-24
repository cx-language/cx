#pragma once

#include <cstdint>
#include <ostream>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)
#include "../ast/location.h"

namespace llvm {
class APSInt;
class APFloat;
template<typename T> class ArrayRef;
} // namespace llvm

namespace cx {

struct Token {
    enum Kind {
        None,
        Newline,
        Identifier,
        IntegerLiteral,
        FloatLiteral,
        StringLiteral,
        CharacterLiteral,
        InterpStart,
        InterpEnd,
        Break,
        Case,
        Const,
        Continue,
        Default,
        Defer,
        Do,
        Else,
        Enum,
        Extern,
        False,
        For,
        If,
        Import,
        In,
        Interface,
        Is,
        Null,
        Private,
        Public,
        Return,
        Sizeof,
        Struct,
        Switch,
        Test,
        Then,
        This,
        True,
        Undefined,
        Using,
        Var,
        While,
        HashIf,
        HashElse,
        HashEndif,
        Equal,
        NotEqual,
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
        PositiveModulo,
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
        FatArrow,
        QuestionMark,
        QuestionQuestion,
        TokenCount
    };

    Token(Token::Kind kind, Location location, llvm::StringRef string = {});
    Token(Location location, uint64_t val, int length);
    operator Token::Kind() const { return kind; }
    llvm::StringRef getString() const { return src.string; }
    bool is(Token::Kind kind) const { return this->kind == kind; }
    bool is(llvm::ArrayRef<Token::Kind> kinds) const;
    llvm::APSInt getIntegerValue() const;
    int getIntegerLength() const { return src.integer.length; }
    llvm::APFloat getFloatingPointValue() const;

    Token::Kind kind;

private:
    struct IntegerValue {
        uint64_t value; ///< The parsed integer literal value (only valid if this is an IntegerLiteral token).
        int length; ///< The length of the literal in the source code.
    };

    union {
        llvm::StringRef string; ///< The substring in the source code representing this token.
        IntegerValue integer; ///< The parsed integer literal value (only valid if this is an IntegerLiteral token).
    } src;

public:
    Location location;
};

struct UnaryOperator {
    UnaryOperator(Token::Kind kind);
    operator Token::Kind() const { return kind; }

    Token::Kind kind;
};

struct BinaryOperator {
    BinaryOperator(Token::Kind kind);
    operator Token::Kind() const { return kind; }

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
llvm::StringRef getFunctionName(Token::Kind tokenKind);

/// Strips the trailing '=' from a compound assignment operator.
/// E.g. given '+=', returns '+', and so on.
inline Token::Kind withoutCompoundEqSuffix(Token::Kind tokenKind) {
    return static_cast<Token::Kind>(static_cast<int>(tokenKind) - 1);
}

const char* toString(Token::Kind tokenKind);
/// Returns the source location just past the token.
Location getTokenEndLocation(const Token& token);
std::ostream& operator<<(std::ostream& stream, Token::Kind tokenKind);
llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, Token::Kind tokenKind);

} // namespace cx
