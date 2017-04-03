#include "token.h"
#include <ostream>
#include <cassert>
#include <cerrno>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/ADT/STLExtras.h>

using namespace delta;

namespace {

enum class PrecedenceGroup {
    LogicalOr,
    LogicalAnd,
    Bitwise,
    Comparison,
    AddSub,
    MulDiv,
};

PrecedenceGroup getPrecedenceGroup(TokenKind tokenKind) {
    switch (tokenKind) {
        case EQ: case NE: return PrecedenceGroup::Comparison;
        case LT: case LE: case GT: case GE: return PrecedenceGroup::Comparison;
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
extern const char* currentFileName;
extern SrcLoc firstLoc;
extern SrcLoc lastLoc;
}

Token::Token(TokenKind kind, llvm::StringRef string)
: kind(kind), string(string), srcLoc(currentFileName, firstLoc.line, firstLoc.column) {
    assert(!string.empty() || kind == NO_TOKEN || kind >= BREAK);
    assert(srcLoc.isValid());
#ifndef NDEBUG
    if (kind == INT_LITERAL) (void) getIntegerValue(); // Validate the integer value.
    if (kind == FLOAT_LITERAL) (void) getFloatingPointValue(); // Validate the FP value.
#endif
}

bool Token::isBinaryOperator() const {
    switch (kind) {
        case EQ: case NE: case LT: case LE: case GT: case GE: case PLUS:
        case MINUS: case STAR: case SLASH: case MOD: case AND: case AND_AND:
        case OR: case OR_OR: case XOR: case LSHIFT: case RSHIFT: return true;
        default: return false;
    }
}

bool Token::isPrefixOperator() const {
    switch (kind) {
        case PLUS: case MINUS: case STAR: case AND: case NOT: case COMPL: return true;
        default: return false;
    }
}

int Token::getPrecedence() const {
    return int(getPrecedenceGroup(kind));
}

int64_t Token::getIntegerValue() const {
    int64_t value;
    bool fail = string.getAsInteger(0, value);
    (void) fail;
    assert(!fail && "invalid integer literal");
    return value;
}

long double Token::getFloatingPointValue() const {
    long double value = std::strtold(string.str().c_str(), nullptr);
    assert(errno != ERANGE && "invalid floating-point literal");
    return value;
}

PrefixOperator::PrefixOperator(Token token) : kind(token) {
    assert(token.isPrefixOperator());
}

BinaryOperator::BinaryOperator(Token token) : kind(token) {
    assert(token.isBinaryOperator());
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

std::ostream& delta::operator<<(std::ostream& stream, TokenKind tokenKind) {
    static const char* const tokenStrings[] = {
        "end-of-file", "identifier", "number", "float literal", "string literal",
        "break", "case", "cast", "class", "const", "default", "defer", "deinit", "else",
        "extern", "false", "func", "if", "import", "init", "mutable", "mutating", "null",
        "return", "struct", "switch", "this", "true", "uninitialized", "var", "while",
        "_", "==", "!=", "<", "<=", ">", ">=", "+", "+=", "-", "-=", "*", "*=",
        "/", "/=", "%", "%=", "++", "--", "!", "&", "&=", "&&", "&&=", "|", "|=", "||", "||=",
        "^", "^=", "~", "<<", "<<=", ">>", ">>=", "=", "(", ")", "[", "]", "{", "}",
        ".", ",", ":", "::", ";", "->",
    };
    static_assert(llvm::array_lengthof(tokenStrings) == TOKEN_COUNT,
                  "tokenStrings array not up-to-date");
    return stream << tokenStrings[tokenKind];
}
