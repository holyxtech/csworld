#version 460 core

uniform mat4 transform;

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uvs;
layout (location = 3) in float layer;

out vec2 fragUvs;
flat out float fragLayer;

void main() {
    gl_Position = transform * vec4(position, 1.0);
    fragUvs = uvs;
    fragLayer = layer;
}