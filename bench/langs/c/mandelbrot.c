// Mandelbrot checksum, matching bench/mandelbrot.cx. Float32 loop counters
// and int32 sum, exactly like the cx version; the printed checksum is only
// self-consistent (float-to-int conversion of huge values is platform-defined).
#include <stdio.h>

typedef struct {
    float r;
    float i;
} Complex;

int main(void) {
    int sum = 0;
    for (float y = -0.9f; y < 0.9f; y += 0.005f) {
        for (float x = -1.4f; x < 0.4f; x += 0.0025f) {
            Complex z = {0.0f, 0.0f};
            for (int k = 0; k <= 1000; k++) {
                float r = z.r * z.r - z.i * z.i + x;
                float i = z.r * z.i + z.i * z.r + y;
                z.r = r;
                z.i = i;
            }
            sum += (int)((z.r * z.r + z.i * z.i) * 50.0f);
        }
    }
    printf("%d\n", sum);
    return 0;
}
