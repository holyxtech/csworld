#version 460 core

uniform sampler2DArray textureArray;

in vec2 fragUvs;
flat in uint fragTextureId;

out vec4 color;

void main() {
  color = texture(textureArray, vec3(fragUvs, fragTextureId));
  if (color.a == 0.0)
    discard; 
}