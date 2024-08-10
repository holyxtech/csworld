#version 460 core

uniform mat4 uTransform; // should be in UBO

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvs;

out vec2 fragUvs;

void main() {
  vec4 pos = uTransform * vec4(position, 1.0); 
  // getting z fighting with the sky when the sun is in the corner unless i do this
  gl_Position = vec4(pos.xy, pos.w-0.000001, pos.w);
  fragUvs = uvs;
}