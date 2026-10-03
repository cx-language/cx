// Prime number sieve, matching sieve.cx.
let count = 100_000_000
var isPrime = [Bool](repeating: true, count: count)
var primes = 0
for i in 2..<count {
    if !isPrime[i] { continue }
    primes += 1
    var j = i + i
    while j < count {
        isPrime[j] = false
        j += i
    }
}
print(primes)
