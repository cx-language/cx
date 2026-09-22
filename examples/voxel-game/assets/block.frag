#version 330 core
in vec3 vColor;
// Centroid sampling keeps edge fragments inside the tile: without it,
// MSAA edge fragments extrapolate UVs into neighbor tiles.
centroid in vec2 vUV;
in float vShade;
in float vDist;
uniform sampler2D blockTex;
uniform vec3 fogColor;
uniform float fogNear;
uniform float fogFar;
out vec4 fragColor;
void main() {
    vec3 base = texture(blockTex, vUV).rgb * vColor * vShade;
    float f = clamp((vDist - fogNear) / (fogFar - fogNear), 0.0, 1.0);
    fragColor = vec4(mix(base, fogColor, f), 1.0);
}
