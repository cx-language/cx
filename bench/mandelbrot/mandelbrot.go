// Mandelbrot checksum, matching mandelbrot.cx. The printed checksum is only
// self-consistent (float-to-int conversion of huge values is platform-defined).
package main

import "fmt"

type Complex struct {
	r, i float64
}

func (z Complex) abs() float64 {
	return z.r*z.r + z.i*z.i
}

func add(a, b Complex) Complex {
	return Complex{a.r + b.r, a.i + b.i}
}

func mul(a, b Complex) Complex {
	return Complex{a.r*b.r - a.i*b.i, a.r*b.i + a.i*b.r}
}

func main() {
	var sum int
	for y := -0.9; y < 0.9; y += 0.005 {
		for x := -1.4; x < 0.4; x += 0.0025 {
			z := Complex{}
			for k := 0; k <= 1000; k++ {
				z = add(mul(z, z), Complex{x, y})
			}
			sum += int(z.abs() * 50)
		}
	}
	fmt.Println(sum)
}
