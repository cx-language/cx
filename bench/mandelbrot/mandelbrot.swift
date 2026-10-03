// Mandelbrot checksum, matching mandelbrot.cx. Only self-consistent:
// float-to-int conversion of huge values is platform-defined.
struct Complex {
    var r: Double
    var i: Double

    func abs() -> Double {
        r * r + i * i
    }
}

func + (a: Complex, b: Complex) -> Complex {
    Complex(r: a.r + b.r, i: a.i + b.i)
}

func * (a: Complex, b: Complex) -> Complex {
    Complex(r: a.r * b.r - a.i * b.i, i: a.r * b.i + a.i * b.r)
}

// Saturating conversion: diverged orbits overflow Int32, which would trap.
// Matches Rust's `as` cast (NaN becomes 0).
func saturatedInt32(_ v: Double) -> Int32 {
    if v.isNaN { return 0 }
    if v >= 2147483648.0 { return .max }
    if v < -2147483648.0 { return .min }
    return Int32(v)
}

var sum: Int32 = 0
var y = -0.9
while y < 0.9 {
    var x = -1.4
    while x < 0.4 {
        var z = Complex(r: 0, i: 0)
        for _ in 0...1000 {
            z = z * z + Complex(r: x, i: y)
        }
        sum &+= saturatedInt32(z.abs() * 50)
        x += 0.0025
    }
    y += 0.005
}
print(sum)
