#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D u_DragTexture;

void main() {
  // flip y texture coordinate
  vec2 flippedTexCoord = vec2(TexCoord.x, 1.0 - TexCoord.y);
  FragColor = texture(u_DragTexture, flippedTexCoord);
  if (FragColor.a < 0.5) {
    discard;
  }
}