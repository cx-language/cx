#version 330 core
// Mandelbrot set with smooth iteration-count coloring.
in vec2 uv;
out vec4 fragColor;
uniform vec2 uCenter;
uniform float uViewH;
uniform float uAspect;
uniform int uMaxIter;
void main() {
    vec2 c = uCenter + vec2((uv.x - 0.5) * uAspect * uViewH, (uv.y - 0.5) * uViewH);
    vec2 z = vec2(0.0, 0.0);
    int i = 0;
    for (; i < uMaxIter; i++) {
        if (dot(z, z) > 4.0) break;
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
    }
    if (i == uMaxIter) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    float logZn = log(dot(z, z)) * 0.5;
    float nu = log(logZn / log(2.0)) / log(2.0);
    float t = float(i) + 1.0 - nu;
    vec3 col = 0.5 + 0.5 * cos(6.28318 * (t * 0.02 + vec3(0.0, 0.33, 0.67)));
    fragColor = vec4(col, 1.0);
}
