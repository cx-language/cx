#include "token.h"
#include <ostream>
#include <cerrno>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/STLExtras.h>
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

PrecedenceGroup getPrecedenceGroup(TokenKind tokenKind) {
    switch (tokenKind) {
        case EQ: case NE: return PrecedenceGroup::Comparison;
        case LT: case LE: case GT: case GE: return PrecedenceGroup::Comparison;
        case DOTDOT: case DOTDOTDOT: return PrecedenceGroup::Range;
        case PLUS: case MINUS: return PrecedenceGroup::AddSub;
        case STAR: case SLASH: case MOD: return PrecedenceGroup::MulDiv;
        case AND_AND: return PrecedenceGroup::LogicalAnd;
        case OR_OR: return PrecedenceGroup::LogicalOr;
        case AND: case OR: case XOR: return PrecedenceGroup::Bitwise;
        case LSHIFT: case RSHIFT: return PrecedenceGroup::Bitwise;
        default: llvm_unreachable("invalid binary operator");
    }
}

}

namespace delta {
const char* currentFilePath;
SourceLocation firstLocation(nullptr, 1, 0);
SourceLocation lastLocation(nullptr, 1, 0);
}

Token::Token(TokenKind kind, llvm::StringRef string)
: kind(kind), string(string), location(currentFilePath, firstLocation.line, firstLocation.column) {
    ASSERT(!string.empty() || kind == NO_TOKEN || kind >= BREAK);
    ASSERT(location.isValid());
#ifndef NDEBUG
    if (kind == INT_LITERAL) (void) getIntegerValue(); // Validate the integer value.
    if (kind == FLOAT_LITERAL) (void) getFloatingPointValue(); // Validate the FP value.
#endif
}

bool Token::isBinaryOperator() const {
    switch (kind) {
        case EQ: case NE: case LT: case LE: case GT: case GE: case PLUS:
        case MINUS: case STAR: case SLASH: case MOD: case AND: case AND_AND:
        case OR: case OR_OR: case XOR: case LSHIFT: case RSHIFT:
        case DOTDOT: case DOTDOTDOT: return true;
        default: return false;
    }
}

bool Token::isPrefixOperator() const {
    switch (kind) {
        case PLUS: case MINUS: case STAR: case AND: case NOT: case COMPL: return true;
        default: return false;
    }
}

bool Token::isAssignmentOperator() const {
    return kind == ASSIGN || isCompoundAssignmentOperator();
}

bool Token::isCompoundAssignmentOperator() const {
    switch (kind) {
        case PLUS_EQ: case MINUS_EQ: case STAR_EQ: case SLASH_EQ: case MOD_EQ:
        case AND_EQ: case AND_AND_EQ: case OR_EQ: case OR_OR_EQ:
        case XOR_EQ: case LSHIFT_EQ: case RSHIFT_EQ: return true;
        default: return false;
    }
}

bool Token::isOverloadable() const {
    switch (kind) {
        case EQ: case LT: case PLUS: case MINUS: case STAR: case SLASH: case MOD: return true;
        default: return false;
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
        case EQ: case NE: case LT: case LE: case GT: case GE: return true;
        default: return false;
    }
}

bool BinaryOperator::isBitwiseOperator() const {
    switch (kind) {
        case AND: case AND_EQ: case OR: case OR_EQ: case XOR: case XOR_EQ: case COMPL:
        case LSHIFT: case LSHIFT_EQ: case RSHIFT: case RSHIFT_EQ: return true;
        default: return false;
    }
}

std::string BinaryOperator::getFunctionName() const {
    switch (kind) {
        case DOTDOT: return "Range";
        case DOTDOTDOT: return "ClosedRange";
        default: return toString(kind);
    }
}

const char* delta::toString(TokenKind tokenKind) {
    static const char* const tokenStrings[] = {
        "end-of-file", "newline", "identifier", "number", "float literal", "string literal",
        "break", "case", "cast", "class", "const", "default", "defer", "deinit", "else",
        "extern", "false", "for", "func", "if", "import", "in", "init", "interface", "let",
        "mutable", "mutating", "null", "return", "sizeof", "struct", "switch", "this", "true",
        "undefined", "var", "while",
        "_", "==", "!=", "<", "<=", ">", ">=", "+", "+=", "-", "-=", "*", "*=",
        "/", "/=", "%", "%=", "++", "--", "!", "&", "&=", "&&", "&&=", "|", "|=", "||", "||=",
        "^", "^=", "~", "<<", "<<=", ">>", ">>=", "=", "(", ")", "[", "]", "{", "}",
        ".", "..", "...", ",", ":", ";", "->", "?"
    };
    static_assert(llvm::array_lengthof(tokenStrings) == TOKEN_COUNT,
                  "tokenStrings array not up-to-date");
    return tokenStrings[tokenKind];
}

std::ostream& delta::operator<<(std::ostream& stream, TokenKind tokenKind) {
    return stream << toString(tokenKind);
}
