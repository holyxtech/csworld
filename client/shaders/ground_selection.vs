#version 460 core
#extension GL_ARB_shading_language_include : require

#include <common.glsl>

layout (location = 0) in vec3 position;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
}