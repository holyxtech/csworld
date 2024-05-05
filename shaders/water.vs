#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvs;
layout (location = 2) in int textureId;
layout (location = 3) in float lighting;

uniform mat4 uTransform;

out vec3 fragWorldPosition;

void main() {
    gl_Position = uTransform * vec4(position, 1.0);
    fragWorldPosition = position;
}