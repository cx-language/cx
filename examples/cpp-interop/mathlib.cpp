#include "mathlib.h"

Calculator::Calculator(double initial) : value(initial) {}

void Calculator::add(double value) {
    this->value += value;
}

void Calculator::multiply(double value) {
    this->value *= value;
}

double Calculator::result() const {
    return value;
}

double factorial(int n) {
    double result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}
