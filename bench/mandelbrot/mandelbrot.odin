// Mandelbrot checksum, matching mandelbrot.cx. The printed checksum is only
// self-consistent (float-to-int conversion of huge values is platform-defined).
package mandelbrot

import "core:fmt"

Complex :: struct {
	r, i: f64,
}

abs :: proc(z: Complex) -> f64 {
	return z.r * z.r + z.i * z.i
}

add :: proc(a, b: Complex) -> Complex {
	return {a.r + b.r, a.i + b.i}
}

mul :: proc(a, b: Complex) -> Complex {
	return {a.r * b.r - a.i * b.i, a.r * b.i + a.i * b.r}
}

main :: proc() {
	sum: int
	for y := -0.9; y < 0.9; y += 0.005 {
		for x := -1.4; x < 0.4; x += 0.0025 {
			z := Complex{}
			for _ in 0 ..= 1000 {
				z = add(mul(z, z), Complex{x, y})
			}
			sum += int(abs(z) * 50)
		}
	}
	fmt.println(sum)
}
