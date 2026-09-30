// Sieve of Eratosthenes below 1e8, matching sieve.cx.
fn main() {
    const COUNT: usize = 100_000_000;
    let mut is_prime = vec![true; COUNT];
    let mut primes: i32 = 0;
    for i in 2..COUNT {
        if !is_prime[i] {
            continue;
        }
        primes += 1;
        let mut j = i + i;
        while j < COUNT {
            is_prime[j] = false;
            j += i;
        }
    }
    println!("{primes}");
}
