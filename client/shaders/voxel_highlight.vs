#version 460 core
#extension GL_ARB_shading_language_include : require

#include <common.glsl>

uniform mat4 model;

layout (location = 0) in vec3 position;

void main() {
    vec4 viewPos =  view * model * vec4(position, 1.0);
    viewPos -= vec4(0,0,.005,0);
    gl_Position =  projection * viewPos;
}