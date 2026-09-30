// Naive recursive Fibonacci, matching bench/fib.cx.
#include <cstdio>

namespace {

int fib(int n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

} // namespace

int main() {
    std::printf("%d\n", fib(40));
}
