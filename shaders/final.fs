#version 460 core

uniform sampler2D scene;
uniform sampler2D bloom;
uniform float exposure = 1.f;

in vec2 texCoords;

out vec4 color;

void main() {
  vec3 hdrColor = texture(scene, texCoords).rgb;
  vec3 bloomColor = texture(bloom, texCoords).rgb;
  const float gamma = 2.2;
  hdrColor += bloomColor; // additive blending
  // tone mapping
  vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
  // also gamma correct while we're at it       
  result = pow(result, vec3(1.0 / gamma));
  color = vec4(result, 1.0);
}