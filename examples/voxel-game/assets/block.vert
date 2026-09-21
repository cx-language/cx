#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
uniform mat4 mvp;
uniform vec3 cameraPos;
out vec3 vColor;
out float vShade;
out float vDist;
void main() {
    gl_Position = mvp * vec4(position, 1.0);
    vColor = color;
    vec3 sunDir = normalize(vec3(0.5, 1.0, 0.3));
    vShade = 0.55 + 0.45 * max(dot(normal, sunDir), 0.0);
    vDist = distance(position, cameraPos);
}
