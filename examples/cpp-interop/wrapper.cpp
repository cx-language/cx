#include "wrapper.h"
#include "mathlib.h"

extern "C" {

CalculatorHandle* calculator_create(double initial) {
    return reinterpret_cast<CalculatorHandle*>(new Calculator(initial));
}

void calculator_add(CalculatorHandle* calc, double value) {
    reinterpret_cast<Calculator*>(calc)->add(value);
}

void calculator_multiply(CalculatorHandle* calc, double value) {
    reinterpret_cast<Calculator*>(calc)->multiply(value);
}

double calculator_result(const CalculatorHandle* calc) {
    return reinterpret_cast<const Calculator*>(calc)->result();
}

void calculator_destroy(CalculatorHandle* calc) {
    delete reinterpret_cast<Calculator*>(calc);
}

double math_factorial(int n) {
    return factorial(n);
}

} // extern "C"
