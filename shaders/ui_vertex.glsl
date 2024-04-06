#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uvs;
layout (location = 2) in int layer;

out vec2 fragUvs;
flat out int fragLayer;
out vec2 fragPosition;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    fragPosition = position;
    fragUvs = uvs;
    fragLayer = layer;
}