#version 460 core

uniform mat4 uTransform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvs;
layout (location = 2) in float layer;
layout (location = 3) in float lighting;

out vec2 fragUvs;
out vec3 fragWorldPosition;
flat out float fragLayer;
out float fragLighting;


void main() {
    gl_Position = uTransform * vec4(position, 1.0);
    fragUvs = uvs;
    fragLayer = layer;
    fragLighting = lighting;
    fragWorldPosition = position;
}