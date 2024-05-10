#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvs;
layout (location = 2) in int textureId;
layout (location = 3) in float lighting;

uniform mat4 uProjection;
uniform mat4 uView;

out vec3 fragWorldPosition;
out vec4 fragCameraPosition;
out vec4 fragCameraNormal;

void main() {
  vec4 cameraPosition = uView * vec4(position, 1.0);

  fragWorldPosition = position;
  fragCameraPosition = cameraPosition;
  
  // for now we just assume the water is flat on the xz plane
  fragCameraNormal = uView * vec4(0.f, 1.f, 0.f, 0.f);

  gl_Position = uProjection * cameraPosition;
}