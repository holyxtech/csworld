#version 460 core

#include <common.glsl>

layout (location = 0) in vec3 position;

void main() {
    vec4 viewPos =  view * model * vec4(position, 1.0);
    viewPos -= vec4(0,0,.005,0);
    gl_Position =  projection * viewPos;
}
