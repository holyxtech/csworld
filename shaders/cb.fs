#version 460 core

in vec2 fragUvs;
uniform sampler2D cbTexture;

out vec4 color;

void main() {
  // the multiplier should be a material input
  color = texture(cbTexture, fragUvs) * vec4(1.5f,1.5f,1.5f,1.0f);
  if (color.a == 0)
    discard;
}
