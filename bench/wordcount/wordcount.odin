// Word frequency counter, matching wordcount.cx: xorshift64 word stream,
// hash map inserts and lookups, order-independent aggregate.
package wordcount

import "core:fmt"
import "core:strings"

syllables := [?]string{
	"al", "be", "cor", "di", "el", "for", "gi", "ha", "il", "jo", "ka", "li", "ma", "no", "or", "pa",
	"qu", "ra", "si", "ta", "ul", "vi", "wo", "xa", "ya", "zo", "ash", "bel", "cam", "dor", "esh", "fim",
}

main :: proc() {
	words := make([dynamic]string, 0, 4_000_000)
	counts := make(map[string]i32)
	state: u64 = 0x12345678
	for _ in 0 ..< 4_000_000 {
		state ~= state << 13
		state ~= state >> 7
		state ~= state << 17
		a := int(state % u64(len(syllables)))
		state ~= state << 13
		state ~= state >> 7
		state ~= state << 17
		b := int(state % u64(len(syllables)))
		parts := [2]string{syllables[a], syllables[b]}
		word := strings.concatenate(parts[:]) or_else panic("out of memory")
		append(&words, word)
		counts[word] += 1
	}
	total: i64
	sum_squares: i64
	for _, count in counts {
		total += i64(count)
		sum_squares += i64(count) * i64(count)
	}
	fmt.println(i64(len(counts)) * 1_000_000_000_000 + total * 1_000_000 + sum_squares)
	for word in words {
		delete(word)
	}
	delete(words)
	delete(counts)
}
