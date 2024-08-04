layout (binding = 2, std140) uniform CommonBlock {
  mat4 view;
  vec3 sunDir;
  float time;
};