// Filter/map pipeline plus capturing closure, matching mapfilter.cx.
const DIVISOR: i32 = 7;
const FACTOR: i32 = 3;

fn main() {
    let mut numbers: Vec<i32> = Vec::with_capacity(4_000_000);
    numbers.extend(0..4_000_000);
    let mut total: i64 = 0;
    for _ in 0..40 {
        for x in numbers
            .iter()
            .copied()
            .filter(|&n| n % DIVISOR == 0)
            .map(|n| n * FACTOR + 1)
            .filter(|&n| n % 2 == 0)
        {
            total += i64::from(x);
        }
    }
    let offset = 1;
    let adjust = |n: i32| n * FACTOR + offset;
    for i in 0..8_000_000 {
        total += i64::from(adjust(i));
    }
    println!("{total}");
}
