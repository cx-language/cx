// A small C++ library with a class and free functions.
// Wrapped as C ABI in wrapper.h/wrapper.cpp so cx can call it.

#pragma once

class Calculator {
public:
    Calculator(double initial);
    void add(double value);
    void multiply(double value);
    double result() const;

private:
    double value;
};

double factorial(int n);
