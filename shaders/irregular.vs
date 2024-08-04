#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvs;
layout (location = 2) in int textureId;

out VS_OUT {
  vec3 worldPos;
  vec3 worldNormal;
} vs_out;
out vec2 fragUvs;
flat out uint fragTextureId;

uniform mat4 uTransform; // should be in UBO

void main() {
    gl_Position = uTransform * vec4(position, 1.0);
    fragUvs = uvs;
    fragTextureId = textureId;
    vs_out.worldPos = position;
    vs_out.worldNormal = vec3(1.f,0.f,0.f);
}