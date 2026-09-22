#version 330 core
in vec3 vColor;
in float vShade;
in float vDist;
uniform vec3 fogColor;
uniform float fogNear;
uniform float fogFar;
out vec4 fragColor;
void main() {
    vec3 base = vColor * vShade;
    float f = clamp((vDist - fogNear) / (fogFar - fogNear), 0.0, 1.0);
    fragColor = vec4(mix(base, fogColor, f), 1.0);
}
