#version 460 core

uniform sampler2D scene;
uniform sampler2D bloom;
uniform float exposure = 1.f;

out vec4 color;

void main() {
  ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
  const float gamma = 2.2;
  vec3 hdrColor = texelFetch(scene, texcoord, 0).rgb;      
  vec3 bloomColor = texelFetch(bloom, texcoord, 0).rgb;
  hdrColor += bloomColor; // additive blending
  // tone mapping
  vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
  // also gamma correct while we're at it       
  result = pow(result, vec3(1.0 / gamma));
  color = vec4(result, 1.0);
}