#version 460 core

uniform sampler2D image;
uniform bool horizontal;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

out vec4 color;

void main() {             
  ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
  vec3 result = texelFetch(image, texcoord, 0 ).rgb * weight[0];
  if (horizontal) {
    for (int i = 1; i < 5; ++i) {
      result += texelFetch(image, texcoord + ivec2(i, 0), 0).rgb * weight[i];
      result += texelFetch(image, texcoord - ivec2(i, 0), 0).rgb * weight[i];
    }
  }
  else {
    for (int i = 1; i < 5; ++i) {
      result += texelFetch(image, texcoord + ivec2(0, i), 0).rgb * weight[i];
      result += texelFetch(image, texcoord - ivec2(0, i), 0).rgb * weight[i];
    }
  } 
  color = vec4(result, 1.0);
}