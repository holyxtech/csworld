#version 460 core

layout (location = 0) in vec2 position;

out vec2 texCoords;

void main() {
    texCoords = (position + vec2(1.f)) / 2;
    gl_Position = vec4(position, 0.0, 1.0);
}