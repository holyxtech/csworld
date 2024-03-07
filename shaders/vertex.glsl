#version 460 core

uniform mat4 uTransform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uvs;
layout (location = 3) in float layer;

out vec2 fragUvs;
out vec3 fragNormal;
flat out float fragLayer;

void main() {
    gl_Position = uTransform * vec4(position, 1.0);
    fragNormal = normal;
    fragUvs = uvs;
    fragLayer = layer;
}