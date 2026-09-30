// Mandelbrot checksum, matching mandelbrot.cx. f32 loop counters and i32
// sum like the cx version; the printed checksum is only self-consistent
// (float-to-int conversion of huge values is platform-defined).
package mandelbrot

import "core:fmt"

Complex :: struct {
	r, i: f32,
}

abs :: proc(z: Complex) -> f32 {
	return z.r * z.r + z.i * z.i
}

add :: proc(a, b: Complex) -> Complex {
	return {a.r + b.r, a.i + b.i}
}

mul :: proc(a, b: Complex) -> Complex {
	return {a.r * b.r - a.i * b.i, a.r * b.i + a.i * b.r}
}

main :: proc() {
	sum: i32
	for y: f32 = -0.9; y < 0.9; y += 0.005 {
		for x: f32 = -1.4; x < 0.4; x += 0.0025 {
			z := Complex{}
			for _ in 0 ..= 1000 {
				z = add(mul(z, z), Complex{x, y})
			}
			sum += i32(abs(z) * 50)
		}
	}
	fmt.println(sum)
}
