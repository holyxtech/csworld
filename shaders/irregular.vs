#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvs;
layout (location = 2) in int textureId;

out VS_OUT {
  vec3 worldPos;
  vec3 worldNormal;
  vec3 cameraPos;
  vec2 uvs;
} vs_out;
flat out uint fragTextureId;

uniform mat4 uView;
uniform mat4 uTransform; // should be in UBO

void main() {
    gl_Position = uTransform * vec4(position, 1.0);
    vs_out.uvs = uvs;
    fragTextureId = textureId;
    vs_out.worldPos = position;
    vs_out.worldNormal = vec3(1.f,0.f,0.f);
    vec4 cameraPos = uView * vec4(position, 1.f);
    vs_out.cameraPos = cameraPos.xyz;
}