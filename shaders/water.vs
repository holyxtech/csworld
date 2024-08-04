#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvs;
layout (location = 2) in int textureId;
layout (location = 3) in float lighting;

out VS_OUT {
  vec3 worldPos;
  vec3 worldNormal;
} vs_out;

uniform mat4 uProjection; // should be in UBO
uniform mat4 uView; // should be in UBO

void main() {
  vec3 normal = vec3(0.f, 1.f, 0.f);
  vs_out.worldPos = position;
  vs_out.worldNormal = normal;

  gl_Position = uProjection * uView * vec4(position, 1.0);
}