// Word frequency counter, matching wordcount.cx: xorshift64 word stream,
// hash map inserts and lookups, order-independent aggregate.
package main

import "fmt"

var syllables = []string{"al", "be", "cor", "di", "el", "for", "gi", "ha", "il", "jo", "ka", "li", "ma", "no",
	"or", "pa", "qu", "ra", "si", "ta", "ul", "vi", "wo", "xa", "ya", "zo", "ash", "bel", "cam", "dor", "esh", "fim"}

func main() {
	words := make([]string, 0, 4000000)
	counts := make(map[string]int)
	state := uint64(0x12345678)
	for range 4000000 {
		state ^= state << 13
		state ^= state >> 7
		state ^= state << 17
		a := state % uint64(len(syllables))
		state ^= state << 13
		state ^= state >> 7
		state ^= state << 17
		b := state % uint64(len(syllables))
		word := syllables[a] + syllables[b]
		words = append(words, word)
		counts[word]++
	}
	var total, sumSquares int64
	for _, count := range counts {
		total += int64(count)
		sumSquares += int64(count) * int64(count)
	}
	fmt.Println(int64(len(counts))*1000000000000 + total*1000000 + sumSquares)
}
