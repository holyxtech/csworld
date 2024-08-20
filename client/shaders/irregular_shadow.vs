#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uvs;
layout (location = 3) in int textureId;

out vec2 TexCoord;
flat out uint TextureId;

void main() {
    gl_Position = vec4(position, 1.0);
    TexCoord = uvs;
    TextureId = textureId;
}