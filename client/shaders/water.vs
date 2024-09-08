#version 460 core

#include <common.glsl>

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uvs;
layout (location = 3) in int textureId;

out VS_OUT {
  vec3 worldPos;
  vec3 worldNormal;
  vec4 cameraPos;
  vec4 cameraNormal;
} vs_out;

uniform mat4 uProjection; // should be in UBO
uniform mat4 uView; // should be in UBO

void main() {
  vs_out.worldPos = position;
  vs_out.worldNormal = normal;
  vs_out.cameraPos = uView * vec4(position, 1.f);
  vs_out.cameraNormal = normalMatrix * vec4(normal,0.f);
  gl_Position = uProjection * uView * vec4(position, 1.0);
}
