// Sieve of Eratosthenes below 1e8, matching sieve.cx.
// vector<char> keeps one byte per flag like the cx version (vector<bool> would
// pack bits and measure something else).
#include <cstdio>
#include <vector>

int main() {
    const std::size_t count = 100000000;
    std::vector<char> isPrime(count, true);
    int primes = 0;
    for (std::size_t i = 2; i < count; i++) {
        if (!isPrime[i]) continue;
        primes++;
        for (std::size_t j = i + i; j < count; j += i) {
            isPrime[j] = false;
        }
    }
    std::printf("%d\n", primes);
}
