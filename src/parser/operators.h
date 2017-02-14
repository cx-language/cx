#pragma once

namespace delta {

struct PrefixOperator {
    PrefixOperator(int op) : rawValue(op) { }
    int rawValue;
};

struct BinaryOperator {
    BinaryOperator(int op) : rawValue(op) { }
    bool isComparisonOperator() const;
    int rawValue;
};

std::ostream& operator<<(std::ostream&, PrefixOperator);
std::ostream& operator<<(std::ostream&, BinaryOperator);

}
