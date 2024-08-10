  #version 460 core
#extension GL_ARB_shading_language_include : require

//#include <common.glsl>
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
layout (location=1) out vec4 gPosition;
layout (location=2) out vec4 gNormal;

uniform sampler2DArray textureArray;
uniform samplerCube skybox;
uniform vec3 uCameraWorldPosition;

/* float fogStart = 200;
float fogEnd = 300;
float fogRange = fogEnd - fogStart;

float LinearFog() { 
  float distanceToFragment = length(fs_in.worldPos-uCameraWorldPosition);
  float fragmentDistanceToEnd = fogEnd - distanceToFragment;
  float fog = clamp(fragmentDistanceToEnd / fogRange, 0.0, 1.0);
  return fog;
} */

void main() {
 /*  vec3 direction_to_sky = fs_in.worldPos - uCameraWorldPosition;
  vec4 skyboxColor = texture(skybox, direction_to_sky); */
  vec4 textureColor = texture(textureArray, vec3(fs_in.uvs, fragTextureId));
  if (textureColor.a == 0.0)
    discard;
  
  float shadow = ShadowCalculation(fs_in.worldPos, fs_in.worldNormal, fs_in.cameraPos);
/*   color.rgb = (1-shadow) * textureColor.rgb;
  color.a = textureColor.a; */
  vec3 lighting = (1-shadow) * textureColor.rgb;
  
  // ...
  //vec3 lighting = textureColor.rgb;


  //gColor = vec4(lighting, textureColor.a);
  gColor = vec4(lighting,1.f);
  gPosition = vec4(fs_in.cameraPos,1.f);
  gNormal = vec4(fs_in.cameraNormal,1.f);
  
  

  //color = texture(shadowMap, vec3(fragUvs, 2));

/*   float fog = LinearFog();
  color = mix(skyboxColor, color, fog);
 */
}