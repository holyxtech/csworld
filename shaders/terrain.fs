#version 460 core
#extension GL_ARB_shading_language_include : require

//#include <common.glsl>
#include <shadow.glsl>

in VS_OUT {
    vec3 worldPos;
    vec3 worldNormal;
} fs_in;
in vec2 fragUvs;
flat in uint fragTextureId;

out vec4 color;

uniform mat4 uView;
uniform sampler2DArray textureArray;
uniform samplerCube skybox;
uniform vec3 uCameraWorldPosition;

float fogStart = 200;
float fogEnd = 300;
float fogRange = fogEnd - fogStart;

/* float LinearFog() { 
  float distanceToFragment = length(fragWorldPosition-uCameraWorldPosition);
  float fragmentDistanceToEnd = fogEnd - distanceToFragment;
  float fog = clamp(fragmentDistanceToEnd / fogRange, 0.0, 1.0);
  return fog;
} */

void main() {
  /* vec3 direction_to_sky = fragWorldPosition - uCameraWorldPosition;
  vec4 skyboxColor = texture(skybox, direction_to_sky); */
  vec4 textureColor = texture(textureArray, vec3(fragUvs, fragTextureId));

  if (textureColor.a == 0.0)
    discard;
  
  vec4 cameraPos = uView * vec4(fs_in.worldPos, 1.f);
  float shadow = ShadowCalculation(fs_in.worldPos, fs_in.worldNormal, cameraPos.xyz);
/*   color.rgb = (1-shadow) * textureColor.rgb;
  color.a = textureColor.a; */
  vec3 lighting = (1-shadow) * textureColor.rgb;
  color = vec4(lighting, textureColor.a);
  //color = texture(shadowMap, vec3(fragUvs, 2));

  //float fog = LinearFog();
  //color = mix(skyboxColor, lightedColor, fog);

}
