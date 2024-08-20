#version 460 core

in vec2 GeoTexCoord;
flat in uint GeoTextureId;

uniform sampler2DArray textureArray;

void main() {
  vec4 textureColor = texture(textureArray, vec3(GeoTexCoord, GeoTextureId));
  if (textureColor.a == 0)
    discard; 
}