// Naive recursive Fibonacci, matching fib.cx.
package fib

import "core:fmt"

fib :: proc(n: i32) -> i32 {
	if n < 2 do return n
	return fib(n - 1) + fib(n - 2)
}

main :: proc() {
	fmt.println(fib(40))
}
