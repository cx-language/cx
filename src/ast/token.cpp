#include "token.h"
#include <ostream>
#include <cerrno>
#pragma warning(push, 0)
#include <llvm/Support/ErrorHandling.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/STLExtras.h>
#pragma warning(pop)
#include "../support/utility.h"

using namespace delta;

namespace {

enum class PrecedenceGroup {
    LogicalOr,
    LogicalAnd,
    Bitwise,
    Comparison,
    Range,
    AddSub,
    MulDiv,
};

PrecedenceGroup getPrecedenceGroup(Token::Kind tokenKind) {
    switch (tokenKind) {
        case Token::Equal: case Token::NotEqual: case Token::PointerEqual: case Token::PointerNotEqual:
            return PrecedenceGroup::Comparison;
        case Token::Less: case Token::LessOrEqual: case Token::Greater: case Token::GreaterOrEqual:
            return PrecedenceGroup::Comparison;
        case Token::DotDot: case Token::DotDotDot:
            return PrecedenceGroup::Range;
        case Token::Plus: case Token::Minus:
            return PrecedenceGroup::AddSub;
        case Token::Star: case Token::Slash: case Token::Modulo:
            return PrecedenceGroup::MulDiv;
        case Token::AndAnd:
            return PrecedenceGroup::LogicalAnd;
        case Token::OrOr:
            return PrecedenceGroup::LogicalOr;
        case Token::And: case Token::Or: case Token::Xor:
            return PrecedenceGroup::Bitwise;
        case Token::LeftShift: case Token::RightShift:
            return PrecedenceGroup::Bitwise;
        default:
            llvm_unreachable("invalid binary operator");
    }
}

}

Token::Token(Token::Kind kind, SourceLocation location, llvm::StringRef string)
: kind(kind), string(string), location(location) {
    ASSERT(!string.empty() || kind == Token::None || kind >= Token::Break);
    ASSERT(location.isValid());
#ifndef NDEBUG
    if (kind == Token::IntegerLiteral) (void) getIntegerValue(); // Validate the integer value.
    if (kind == Token::FloatLiteral) (void) getFloatingPointValue(); // Validate the FP value.
#endif
}

bool Token::isBinaryOperator() const {
    switch (kind) {
        case Token::Equal: case Token::NotEqual: case Token::PointerEqual: case Token::PointerNotEqual:
        case Token::Less: case Token::LessOrEqual: case Token::Greater: case Token::GreaterOrEqual:
        case Token::Plus: case Token::Minus: case Token::Star: case Token::Slash: case Token::Modulo:
        case Token::And: case Token::AndAnd: case Token::Or: case Token::OrOr: case Token::Xor:
        case Token::LeftShift: case Token::RightShift: case Token::DotDot: case Token::DotDotDot:
            return true;
        default:
            return false;
    }
}

bool Token::isPrefixOperator() const {
    switch (kind) {
        case Token::Plus: case Token::Minus: case Token::Star: case Token::And: case Token::Not:
        case Token::Tilde:
            return true;
        default:
            return false;
    }
}

bool Token::isAssignmentOperator() const {
    return kind == Token::Assignment || isCompoundAssignmentOperator();
}

bool Token::isCompoundAssignmentOperator() const {
    switch (kind) {
        case Token::PlusEqual: case Token::MinusEqual: case Token::StarEqual: case Token::SlashEqual:
        case Token::ModuloEqual: case Token::AndEqual: case Token::AndAndEqual: case Token::OrEqual:
        case Token::OrOrEqual: case Token::XorEqual: case Token::LeftShiftEqual: case Token::RightShiftEqual:
            return true;
        default:
            return false;
    }
}

bool Token::isOverloadable() const {
    switch (kind) {
        case Token::Equal: case Token::NotEqual: case Token::Less: case Token::LessOrEqual:
        case Token::Greater: case Token::GreaterOrEqual: case Token::Plus: case Token::Minus:
        case Token::Star: case Token::Slash: case Token::Modulo:
            return true;
        default:
            return false;
    }
}

int Token::getPrecedence() const {
    return int(getPrecedenceGroup(kind));
}

int64_t Token::getIntegerValue() const {
    int64_t value;
    bool fail = string.getAsInteger(0, value);
    ASSERT(!fail, "invalid integer literal");
    return value;
}

long double Token::getFloatingPointValue() const {
    long double value = std::strtold(string.str().c_str(), nullptr);
    ASSERT(errno != ERANGE, "invalid floating-point literal");
    return value;
}

PrefixOperator::PrefixOperator(Token token) : kind(token) {
    ASSERT(token.isPrefixOperator());
}

BinaryOperator::BinaryOperator(Token token) : kind(token) {
    ASSERT(token.isBinaryOperator());
}

bool BinaryOperator::isComparisonOperator() const {
    switch (kind) {
        case Token::Equal: case Token::NotEqual: case Token::PointerEqual:
        case Token::PointerNotEqual: case Token::Less: case Token::LessOrEqual:
        case Token::Greater: case Token::GreaterOrEqual:
            return true;
        default:
            return false;
    }
}

bool BinaryOperator::isBitwiseOperator() const {
    switch (kind) {
        case Token::And: case Token::AndEqual: case Token::Or: case Token::OrEqual: case Token::Xor:
        case Token::XorEqual: case Token::Tilde: case Token::LeftShift: case Token::LeftShiftEqual:
        case Token::RightShift: case Token::RightShiftEqual:
            return true;
        default:
            return false;
    }
}

std::string BinaryOperator::getFunctionName() const {
    switch (kind) {
        case Token::DotDot: return "Range";
        case Token::DotDotDot: return "ClosedRange";
        default: return toString(kind);
    }
}

const char* delta::toString(Token::Kind tokenKind) {
    static const char* const tokenStrings[] = {
        "end-of-file", "newline", "identifier", "number", "float literal", "string literal",
        "character literal", "addressof", "break", "case", "cast", "class", "const", "continue",
        "default", "defer", "deinit", "else", "enum", "extern", "false", "for", "func", "if",
        "import", "in", "init", "interface", "let", "mutable", "mutating", "null", "return",
        "sizeof", "struct", "switch", "this", "true", "undefined", "var", "while", "_", "#if",
        "#else", "#endif",
        "==", "!=", "===", "!==", "<", "<=", ">", ">=", "+", "+=", "-", "-=", "*", "*=",
        "/", "/=", "%", "%=", "++", "--", "!", "&", "&=", "&&", "&&=", "|", "|=", "||", "||=",
        "^", "^=", "~", "<<", "<<=", ">>", ">>=", "=", "(", ")", "[", "]", "{", "}",
        ".", "..", "...", ",", ":", ";", "->", "?"
    };
    static_assert(llvm::array_lengthof(tokenStrings) == int(Token::TokenCount),
                  "tokenStrings array not up-to-date");
    return tokenStrings[int(tokenKind)];
}

std::ostream& delta::operator<<(std::ostream& stream, Token::Kind tokenKind) {
    return stream << toString(tokenKind);
}
