// Filter/map pipeline plus capturing closure, matching bench/mapfilter.cx.
// Lazy iterator chain over rangefuncs, collected once per round like .toList().
package main

import (
	"fmt"
	"iter"
	"slices"
)

const divisor = 7
const factor = 3

func filter[T any](seq iter.Seq[T], keep func(T) bool) iter.Seq[T] {
	return func(yield func(T) bool) {
		for x := range seq {
			if keep(x) && !yield(x) {
				return
			}
		}
	}
}

func mapSlice[T, U any](seq iter.Seq[T], f func(T) U) iter.Seq[U] {
	return func(yield func(U) bool) {
		for x := range seq {
			if !yield(f(x)) {
				return
			}
		}
	}
}

func main() {
	numbers := make([]int32, 0, 4000000)
	for i := int32(0); i < 4000000; i++ {
		numbers = append(numbers, i)
	}
	var total int64
	for range 40 {
		selected := slices.Collect(filter(mapSlice(filter(slices.Values(numbers),
			func(n int32) bool { return n%divisor == 0 }),
			func(n int32) int32 { return n*factor + 1 }),
			func(n int32) bool { return n%2 == 0 }))
		for _, x := range selected {
			total += int64(x)
		}
	}
	offset := int32(1)
	adjust := func(n int32) int32 { return n*factor + offset }
	for i := int32(0); i < 8000000; i++ {
		total += int64(adjust(i))
	}
	fmt.Println(total)
}
