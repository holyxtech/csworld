#version 460 core

#include <common.glsl>

layout (location = 0) in vec3 position;

void main() {
    vec4 cameraPos = view * model * vec4(position, 1.0);
    float d = length(cameraPos);
    vec4 adjusted_pos = vec4(position.x, position.y + d/1000, position.z, 1.0);
    gl_Position = projection * view * model * adjusted_pos;
}
