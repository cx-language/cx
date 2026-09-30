// Mandelbrot checksum, matching bench/mandelbrot.cx. Float32 loop counters and
// int32 sum like the cx version; the printed checksum is only self-consistent
// (float-to-int conversion of huge values is platform-defined).
#include <cstdio>

namespace {

struct Complex {
    float r;
    float i;

    float abs() const { return r * r + i * i; }
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
    for (float y = -0.9f; y < 0.9f; y += 0.005f) {
        for (float x = -1.4f; x < 0.4f; x += 0.0025f) {
            Complex z{0.0f, 0.0f};
            for (int k = 0; k <= 1000; k++) {
                z = z * z + Complex{x, y};
            }
            sum += (int)(z.abs() * 50.0f);
        }
    }
    std::printf("%d\n", sum);
}
