#version 460 core

#include <shadow.glsl>

in VS_OUT {
    vec3 worldPos;
    vec3 worldNormal;
    vec3 cameraPos;
    vec3 cameraNormal;
    vec2 uvs;
} fs_in;
flat in uint fragTextureId;

layout (location=0) out vec4 gColor;
layout (location=1) out vec4 gNormal;

uniform sampler2DArray textureArray;

void main() {
  vec4 textureColor = texture(textureArray, vec3(fs_in.uvs, fragTextureId));
  if (textureColor.a == 0.0)
    discard;

  float shadow;
  if (dot(fs_in.worldNormal, lightDir) <= 0)
      shadow = shadowMagnitude;
  else 
      shadow = ShadowCalculation(fs_in.worldPos, fs_in.worldNormal, fs_in.cameraPos);
  vec3 lighting = (1-shadow) * textureColor.rgb;
  gColor = vec4(lighting,1.f);
  gNormal = vec4(fs_in.cameraNormal,1.f);
}
