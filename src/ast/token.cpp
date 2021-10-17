#include "token.h"
#include <cerrno>
#include <ostream>
#pragma warning(push, 0)
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "../support/utility.h"

using namespace cx;

namespace {
enum class PrecedenceGroup {
    Assignment,
    IfExpr,
    LogicalOr,
    LogicalAnd,
    Bitwise,
    Comparison,
    Range,
    AddSub,
    MulDiv,
};
}

static PrecedenceGroup getPrecedenceGroup(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::Equal:
        case Token::NotEqual:
            return PrecedenceGroup::Comparison;
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual:
            return PrecedenceGroup::Comparison;
        case Token::DotDot:
        case Token::DotDotDot:
            return PrecedenceGroup::Range;
        case Token::Plus:
        case Token::Minus:
            return PrecedenceGroup::AddSub;
        case Token::Star:
        case Token::Slash:
        case Token::Modulo:
            return PrecedenceGroup::MulDiv;
        case Token::AndAnd:
            return PrecedenceGroup::LogicalAnd;
        case Token::OrOr:
            return PrecedenceGroup::LogicalOr;
        case Token::And:
        case Token::Or:
        case Token::Xor:
            return PrecedenceGroup::Bitwise;
        case Token::LeftShift:
        case Token::RightShift:
            return PrecedenceGroup::Bitwise;
        case Token::QuestionMark:
            return PrecedenceGroup::IfExpr;
        default:
            if (isAssignmentOperator(tokenKind)) return PrecedenceGroup::Assignment;
            llvm_unreachable("invalid binary operator");
    }
}

Token::Token(Token::Kind kind, SourceLocation location, llvm::StringRef string) : kind(kind), src { string }, location(location) {
    ASSERT(!string.empty() || kind == Token::None || kind >= Token::Break);
    ASSERT(location.isValid());
}

Token::Token(SourceLocation location, uint64_t val) : kind(Token::IntegerLiteral), src { "" }, location(location) {
    ASSERT(location.isValid());
    src.integer = val;
}

bool cx::isBinaryOperator(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::Equal:
        case Token::NotEqual:
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual:
        case Token::Plus:
        case Token::Minus:
        case Token::Star:
        case Token::Slash:
        case Token::Modulo:
        case Token::And:
        case Token::AndAnd:
        case Token::Or:
        case Token::OrOr:
        case Token::Xor:
        case Token::LeftShift:
        case Token::RightShift:
        case Token::DotDot:
        case Token::DotDotDot:
            return true;
        default:
            return isAssignmentOperator(tokenKind);
    }
}

bool cx::isUnaryOperator(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::Plus:
        case Token::Minus:
        case Token::Star:
        case Token::And:
        case Token::Not:
        case Token::Tilde:
        case Token::Increment:
        case Token::Decrement:
            return true;
        default:
            return false;
    }
}

bool cx::isAssignmentOperator(Token::Kind tokenKind) {
    return tokenKind == Token::Assignment || isCompoundAssignmentOperator(tokenKind);
}

bool cx::isCompoundAssignmentOperator(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::PlusEqual:
        case Token::MinusEqual:
        case Token::StarEqual:
        case Token::SlashEqual:
        case Token::ModuloEqual:
        case Token::AndEqual:
        case Token::AndAndEqual:
        case Token::OrEqual:
        case Token::OrOrEqual:
        case Token::XorEqual:
        case Token::LeftShiftEqual:
        case Token::RightShiftEqual:
            return true;
        default:
            return false;
    }
}

bool cx::isOverloadable(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::Equal:
        case Token::NotEqual:
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual:
        case Token::Plus:
        case Token::Minus:
        case Token::Star:
        case Token::Slash:
        case Token::Modulo:
            return true;
        default:
            return false;
    }
}

int cx::getPrecedence(Token::Kind tokenKind) {
    return int(getPrecedenceGroup(tokenKind));
}

bool Token::is(llvm::ArrayRef<Token::Kind> kinds) const {
    return llvm::is_contained(kinds, kind);
}

llvm::APSInt Token::getIntegerValue() const {
    // Avoid overflow with very large values by adding an extra
    // storage bit if the high bit in the source value is set
    llvm::APSInt value(64 + !!(src.integer & (1ULL << 63)), false);
    value = src.integer;
    return value;
}

llvm::APFloat Token::getFloatingPointValue() const {
    // TODO: Which float semantics to use?
    llvm::APFloat value(llvm::APFloat::IEEEsingle(), src.string);
    return value;
}

UnaryOperator::UnaryOperator(Token::Kind kind) : kind(kind) {
    ASSERT(isUnaryOperator(kind));
}

BinaryOperator::BinaryOperator(Token::Kind kind) : kind(kind) {
    ASSERT(isBinaryOperator(kind));
}

bool cx::isComparisonOperator(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::Equal:
        case Token::NotEqual:
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual:
            return true;
        default:
            return false;
    }
}

bool cx::isBitwiseOperator(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::And:
        case Token::AndEqual:
        case Token::Or:
        case Token::OrEqual:
        case Token::Xor:
        case Token::XorEqual:
        case Token::Tilde:
        case Token::LeftShift:
        case Token::LeftShiftEqual:
        case Token::RightShift:
        case Token::RightShiftEqual:
            return true;
        default:
            return false;
    }
}

std::string cx::getFunctionName(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::DotDot:
            return "Range";
        case Token::DotDotDot:
            return "ClosedRange";
        default:
            return toString(tokenKind);
    }
}

const char* cx::toString(Token::Kind tokenKind) {
    static const char* const tokenStrings[] = {
        "end-of-file",
        "newline",
        "identifier",
        "number",
        "float literal",
        "string literal",
        "character literal",
        "as",
        "break",
        "case",
        "const",
        "continue",
        "default",
        "defer",
        "else",
        "enum",
        "extern",
        "false",
        "for",
        "if",
        "import",
        "in",
        "interface",
        "null",
        "private",
        "public",
        "return",
        "sizeof",
        "struct",
        "switch",
        "this",
        "true",
        "undefined",
        "var",
        "while",
        "#if",
        "#else",
        "#endif",
        "==",
        "!=",
        "<",
        "<=",
        ">",
        ">=",
        "+",
        "+=",
        "-",
        "-=",
        "*",
        "*=",
        "/",
        "/=",
        "%",
        "%=",
        "++",
        "--",
        "!",
        "&",
        "&=",
        "&&",
        "&&=",
        "|",
        "|=",
        "||",
        "||=",
        "^",
        "^=",
        "~",
        "<<",
        "<<=",
        ">>",
        ">>=",
        "=",
        "(",
        ")",
        "[",
        "]",
        "{",
        "}",
        ".",
        "..",
        "...",
        ",",
        ":",
        ";",
        "->",
        "?",
    };
    static_assert(llvm::array_lengthof(tokenStrings) == int(Token::TokenCount), "tokenStrings array not up-to-date");
    return tokenStrings[int(tokenKind)];
}

std::ostream& cx::operator<<(std::ostream& stream, Token::Kind tokenKind) {
    return stream << toString(tokenKind);
}

llvm::raw_ostream& cx::operator<<(llvm::raw_ostream& stream, Token::Kind tokenKind) {
    return stream << toString(tokenKind);
}
