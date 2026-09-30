// Sieve of Eratosthenes below 1e8, matching sieve.cx.
// vector<char> keeps one byte per flag like the cx version (vector<bool> would
// pack bits and measure something else).
#include <cstdio>
#include <vector>

int main() {
    const int count = 100000000;
    std::vector<char> isPrime((size_t)count, true);
    int primes = 0;
    for (int i = 2; i < count; i++) {
        if (!isPrime[(size_t)i]) continue;
        primes++;
        for (int j = i + i; j < count; j += i) {
            isPrime[(size_t)j] = false;
        }
    }
    std::printf("%d\n", primes);
}
