// Mandelbrot checksum, matching mandelbrot.cx. float32 loop counters and
// int32 sum like the cx version; the printed checksum is only self-consistent
// (float-to-int conversion of huge values is platform-defined).
package main

import "fmt"

type Complex struct {
	r, i float32
}

func (z Complex) abs() float32 {
	return z.r*z.r + z.i*z.i
}

func add(a, b Complex) Complex {
	return Complex{a.r + b.r, a.i + b.i}
}

func mul(a, b Complex) Complex {
	return Complex{a.r*b.r - a.i*b.i, a.r*b.i + a.i*b.r}
}

func main() {
	var sum int32
	for y := float32(-0.9); y < 0.9; y += 0.005 {
		for x := float32(-1.4); x < 0.4; x += 0.0025 {
			z := Complex{}
			for k := 0; k <= 1000; k++ {
				z = add(mul(z, z), Complex{x, y})
			}
			sum += int32(z.abs() * 50)
		}
	}
	fmt.Println(sum)
}
