layout (binding = 0, std140) uniform CommonBlock {
  mat4 view;
  mat4 projection;
  mat4 invProjection;
  mat4 normalMatrix;
  int frame;
};
uniform mat4 model;