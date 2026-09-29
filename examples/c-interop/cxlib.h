// Declarations of the cx functions exported from cxlib.cx.

#pragma once

int cx_is_even(int n);
double cx_hypot(double a, double b);

typedef struct Big {
    long a, b, c;
} Big;
long cx_sum_big(Big b);
