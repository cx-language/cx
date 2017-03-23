#include "token.h"
#include <ostream>
#include <cassert>
#include <llvm/Support/ErrorHandling.h>

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
        case STAR: case SLASH: return PrecedenceGroup::MulDiv;
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

Token::Token(TokenKind kind)
: kind(kind), srcLoc(currentFileName, firstLoc.line, firstLoc.column) {
    assert(kind != NUMBER);
    assert(srcLoc.isValid());
}

Token::Token(long long number)
: kind(NUMBER), number(number), srcLoc(currentFileName, firstLoc.line, firstLoc.column) {
    assert(srcLoc.isValid());
}

Token::Token(TokenKind kind, char* value)
: kind(kind), string(value), srcLoc(currentFileName, firstLoc.line, firstLoc.column) {
    assert(kind == IDENTIFIER || kind == STRING_LITERAL);
    assert(srcLoc.isValid());
}

bool Token::isBinaryOperator() const {
    switch (kind) {
        case EQ: case NE: case LT: case LE: case GT: case GE: case PLUS:
        case MINUS: case STAR: case SLASH: case AND: case AND_AND:
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

std::ostream& delta::operator<<(std::ostream& stream, TokenKind tokenKind) {
    static const char* const tokenStrings[] = {
        "end-of-file", "identifier", "number", "string literal", "break", "case", "cast",
        "class", "const", "default", "defer", "deinit", "else", "extern", "false",
        "func", "if", "import", "init", "mutable", "null", "return", "struct",
        "switch", "this", "true", "uninitialized", "var", "while",
        "_", "==", "!=", "<", "<=", ">", ">=", "+", "+=", "-", "-=", "*", "*=",
        "/", "/=", "++", "--", "!", "&", "&=", "&&", "&&=", "|", "|=", "||", "||=",
        "^", "^=", "~", "<<", "<<=", ">>", ">>=", "=", "(", ")", "[", "]", "{", "}",
        ".", ",", ":", "::", ";", "->",
    };
    return stream << tokenStrings[tokenKind];
}
