layout (binding = 0, std140) uniform CommonBlock {
  mat4 view;
  mat4 projection;
  mat4 normalMatrix;
};
uniform mat4 model;