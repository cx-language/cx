// Sieve of Eratosthenes below 1e8, matching sieve.cx.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const int count = 100000000;
    bool* isPrime = malloc((size_t)count * sizeof(bool));
    memset(isPrime, true, (size_t)count * sizeof(bool));
    int primes = 0;
    for (int i = 2; i < count; i++) {
        if (!isPrime[i]) continue;
        primes++;
        for (int j = i + i; j < count; j += i) {
            isPrime[j] = false;
        }
    }
    printf("%d\n", primes);
    free(isPrime);
    return 0;
}
