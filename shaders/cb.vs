#version 460 core

uniform mat4 uTransform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvs;

out vec2 fragUvs;

void main() {
  vec4 pos = uTransform * vec4(position, 1.0); 
  gl_Position = pos.xyww;
  fragUvs = uvs;
}