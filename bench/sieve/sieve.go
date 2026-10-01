// Sieve of Eratosthenes below 1e8, matching sieve.cx.
package main

import "fmt"

func main() {
	const count = 100000000
	isPrime := make([]bool, count)
	for i := range isPrime {
		isPrime[i] = true
	}
	var primes int
	for i := 2; i < count; i++ {
		if !isPrime[i] {
			continue
		}
		primes++
		for j := i + i; j < count; j += i {
			isPrime[j] = false
		}
	}
	fmt.Println(primes)
}
