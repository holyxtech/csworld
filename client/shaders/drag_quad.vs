#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec2 u_DragPosition;
uniform vec2 u_DragSize;

out vec2 TexCoord;

void main() {
  // scale the position by the drag size
  vec2 scaledPos = aPos * u_DragSize;
  vec2 finalPos = scaledPos + u_DragPosition;
  gl_Position = vec4(finalPos, 0.0, 1.0);
  TexCoord = aTexCoord;
}