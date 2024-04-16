#version 460 core

in vec2 fragUvs;
uniform sampler2D cbTexture;

out vec4 color;

void main() {
  // the multiplier should be a material input
  color = texture(cbTexture, fragUvs) * vec4(5.5f,5.5f,5.5f,1.f);
}