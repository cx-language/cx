#include "mathlib.h"

double c_add(double a, double b) {
    return a + b;
}

double c_multiply(double a, double b) {
    return a * b;
}

double c_factorial(int n) {
    double result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}
