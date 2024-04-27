#version 460 core

uniform sampler2DArray textureArray;
uniform samplerCube skybox;
uniform vec3 uCameraWorldPosition;

in vec2 fragUvs;
flat in float fragLayer;
flat in float fragLighting;
in vec3 fragWorldPosition;

out vec4 color;

float fogStart = 200;
float fogEnd = 300;
float fogRange = fogEnd - fogStart;

float LinearFog() { 
  float distanceToFragment = length(fragWorldPosition-uCameraWorldPosition);
  float fragmentDistanceToEnd = fogEnd - distanceToFragment;
  float fog = clamp(fragmentDistanceToEnd / fogRange, 0.0, 1.0);
  return fog;
}

void main() {
  vec3 direction_to_sky = fragWorldPosition - uCameraWorldPosition;
  vec4 skyboxColor = texture(skybox, direction_to_sky);
  vec4 textureColor = texture(textureArray, vec3(fragUvs, fragLayer));
  vec4 lightedColor = vec4(textureColor.rgb * fragLighting, textureColor.a);

  if (textureColor.a == 0.0)
    discard;
  
  //float fog = LinearFog();
  //color = mix(skyboxColor, lightedColor, fog);
  color = lightedColor;
}
