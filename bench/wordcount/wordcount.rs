// Word frequency counter, matching wordcount.cx: xorshift64 word stream,
// hash map inserts and lookups, order-independent aggregate.
use std::collections::HashMap;

const SYLLABLES: [&str; 32] = [
    "al", "be", "cor", "di", "el", "for", "gi", "ha", "il", "jo", "ka", "li", "ma", "no", "or",
    "pa", "qu", "ra", "si", "ta", "ul", "vi", "wo", "xa", "ya", "zo", "ash", "bel", "cam", "dor",
    "esh", "fim",
];

fn main() {
    let mut words: Vec<String> = Vec::with_capacity(4_000_000);
    let mut counts: HashMap<String, i32> = HashMap::new();
    let mut state: u64 = 0x12345678;
    for _ in 0..4_000_000 {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        let a = state as usize % SYLLABLES.len();
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        let b = state as usize % SYLLABLES.len();
        let mut word = String::with_capacity(6);
        word.push_str(SYLLABLES[a]);
        word.push_str(SYLLABLES[b]);
        words.push(word);
        let key = words.last().unwrap();
        if let Some(count) = counts.get_mut(key) {
            *count += 1;
        } else {
            counts.insert(key.clone(), 1);
        }
    }
    let mut total: i64 = 0;
    let mut sum_squares: i64 = 0;
    for count in counts.values() {
        let count = i64::from(*count);
        total += count;
        sum_squares += count * count;
    }
    println!(
        "{}",
        counts.len() as i64 * 1_000_000_000_000 + total * 1_000_000 + sum_squares
    );
}
