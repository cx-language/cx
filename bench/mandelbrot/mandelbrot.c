// Mandelbrot checksum, matching mandelbrot.cx. The printed checksum is only
// self-consistent (float-to-int conversion of huge values is platform-defined).
#include <stdio.h>

typedef struct {
    double r;
    double i;
} Complex;

int main(void) {
    int sum = 0;
    for (double y = -0.9; y < 0.9; y += 0.005) {
        for (double x = -1.4; x < 0.4; x += 0.0025) {
            Complex z = {0.0, 0.0};
            for (int k = 0; k <= 1000; k++) {
                double r = z.r * z.r - z.i * z.i + x;
                double i = z.r * z.i + z.i * z.r + y;
                z.r = r;
                z.i = i;
            }
            sum += (int)((z.r * z.r + z.i * z.i) * 50.0);
        }
    }
    printf("%d\n", sum);
    return 0;
}
