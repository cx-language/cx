// Naive recursive Fibonacci, matching fib.cx.
#include <stdio.h>

static int fib(int n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

int main(void) {
    printf("%d\n", fib(40));
    return 0;
}
