// Word frequency counter, matching wordcount.cx.
let syllables = ["al", "be", "cor", "di", "el", "for", "gi", "ha", "il", "jo", "ka", "li", "ma", "no", "or", "pa",
    "qu", "ra", "si", "ta", "ul", "vi", "wo", "xa", "ya", "zo", "ash", "bel", "cam", "dor", "esh", "fim"]
var state: UInt64 = 0x12345678
var words: [String] = []
words.reserveCapacity(4_000_000)
var counts: [String: Int] = [:]
for _ in 0..<4_000_000 {
    // xorshift64, matching the other ports exactly.
    state ^= state << 13
    state ^= state >> 7
    state ^= state << 17
    let a = Int(state % UInt64(syllables.count))
    state ^= state << 13
    state ^= state >> 7
    state ^= state << 17
    let b = Int(state % UInt64(syllables.count))
    let word = syllables[a] + syllables[b]
    words.append(word)
    counts[word, default: 0] += 1
}
// Order-independent aggregate: distinct words, total words, sum of squares.
var total: Int64 = 0
var sumSquares: Int64 = 0
for count in counts.values {
    total += Int64(count)
    sumSquares += Int64(count) * Int64(count)
}
print(Int64(counts.count) * 1_000_000_000_000 + total * 1_000_000 + sumSquares)
