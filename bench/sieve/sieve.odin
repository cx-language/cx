// Sieve of Eratosthenes below 1e8, matching sieve.cx.
package sieve

import "core:fmt"
import "core:slice"

main :: proc() {
	count :: 100_000_000
	is_prime := make([]bool, count)
	slice.fill(is_prime, true)
	primes: i32
	for i: i32 = 2; i < count; i += 1 {
		if !is_prime[i] do continue
		primes += 1
		for j := i + i; j < count; j += i {
			is_prime[j] = false
		}
	}
	fmt.println(primes)
	delete(is_prime)
}
