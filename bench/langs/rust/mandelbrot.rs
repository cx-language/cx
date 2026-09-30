// Mandelbrot checksum, matching bench/mandelbrot.cx. f32 loop counters and i32
// sum like the cx version; the printed checksum is only self-consistent
// (float-to-int conversion of huge values is platform-defined, and `as`
// saturates where C-family conversions wrap).
use std::ops::{Add, Mul};

#[derive(Clone, Copy)]
struct Complex {
    r: f32,
    i: f32,
}

impl Complex {
    fn abs(self) -> f32 {
        self.r * self.r + self.i * self.i
    }
}

impl Add for Complex {
    type Output = Complex;
    fn add(self, b: Complex) -> Complex {
        Complex {
            r: self.r + b.r,
            i: self.i + b.i,
        }
    }
}

impl Mul for Complex {
    type Output = Complex;
    fn mul(self, b: Complex) -> Complex {
        Complex {
            r: self.r * b.r - self.i * b.i,
            i: self.r * b.i + self.i * b.r,
        }
    }
}

fn main() {
    let mut sum: i32 = 0;
    let mut y = -0.9f32;
    while y < 0.9 {
        let mut x = -1.4f32;
        while x < 0.4 {
            let mut z = Complex { r: 0.0, i: 0.0 };
            for _ in 0..=1000 {
                z = z * z + Complex { r: x, i: y };
            }
            sum = sum.wrapping_add((z.abs() * 50.0) as i32);
            x += 0.0025;
        }
        y += 0.005;
    }
    println!("{sum}");
}
