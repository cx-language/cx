#include "operators.h"

bool BinaryOperator::isComparisonOperator() const {
    switch (rawValue) {
        case EQ: case NE: case LT: case LE: case GT: case GE: return true;
        default: return false;
    }
}

std::ostream& delta::operator<<(std::ostream& out, PrefixOperator op) {
    switch (op.rawValue) {
        case PLUS:  return out << "+";
        case MINUS: return out << "-";
        case STAR:  return out << "*";
        case AND:   return out << "&";
        case NOT:   return out << "!";
        case COMPL: return out << "~";
        default: assert(false);
    }
}

std::ostream& delta::operator<<(std::ostream& out, BinaryOperator op) {
    switch (op.rawValue) {
        case EQ:    return out << "==";
        case NE:    return out << "!=";
        case LT:    return out << "<";
        case LE:    return out << "<=";
        case GT:    return out << ">";
        case GE:    return out << ">=";
        case PLUS:  return out << "+";
        case MINUS: return out << "-";
        case STAR:  return out << "*";
        case SLASH: return out << "/";
        case AND:   return out << "&";
        case OR:    return out << "|";
        case XOR:   return out << "^";
        case AND_AND: return out << "&&";
        case OR_OR:   return out << "||";
        case LSHIFT:  return out << "<<";
        case RSHIFT:  return out << ">>";
        default: assert(false);
    }
}
