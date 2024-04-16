#version 460 core

uniform mat4 uTransform;

layout (location = 0) in vec3 position;

void main() {
    gl_Position = uTransform * vec4(position, 1.0);
}