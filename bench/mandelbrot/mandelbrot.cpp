// Mandelbrot checksum, matching mandelbrot.cx. The printed checksum is only
// self-consistent (float-to-int conversion of huge values is platform-defined).
#include <cstdio>

namespace {

struct Complex {
    double r;
    double i;

    double abs() const { return r * r + i * i; }
};

Complex operator+(const Complex& a, const Complex& b) {
    return {a.r + b.r, a.i + b.i};
}

Complex operator*(const Complex& a, const Complex& b) {
    return {a.r * b.r - a.i * b.i, a.r * b.i + a.i * b.r};
}

} // namespace

int main() {
    int sum = 0;
    for (double y = -0.9; y < 0.9; y += 0.005) {
        for (double x = -1.4; x < 0.4; x += 0.0025) {
            Complex z{0.0, 0.0};
            for (int k = 0; k <= 1000; k++) {
                z = z * z + Complex{x, y};
            }
            sum += static_cast<int>(z.abs() * 50.0);
        }
    }
    std::printf("%d\n", sum);
}
