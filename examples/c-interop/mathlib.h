// A small C library. cx imports this header directly (see main.cx).

#pragma once

double c_add(double a, double b);
double c_multiply(double a, double b);
double c_factorial(int n);

typedef struct Big {
    long a, b, c;
} Big;
long c_sum_big(Big b);
