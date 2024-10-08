#version 460 core

#include <common.glsl>
#include <ssr.glsl>

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

uniform sampler2D mainColor;
uniform sampler2D mainDepth;
uniform sampler2D waterColor;
uniform sampler2D waterDepth;
uniform sampler2D waterCameraPosition;
uniform sampler2D waterCameraNormal;
uniform mat4 pixelProjection;

void main() {
  ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
  vec4 fragMainDepth = texelFetch(mainDepth, texcoord, 0);
  vec4 fragWaterDepth = texelFetch(waterDepth, texcoord, 0);
  vec4 fragMainColor = texelFetch(mainColor, texcoord, 0);
  //float AmbientOcclusion = texelFetch(ssao, texcoord, 0).r;
    
  if (fragMainDepth.r <= fragWaterDepth.r) {
    // need to do this elsewhere, so we don't have to occlude everything in main buffer
    color = fragMainColor;// * AmbientOcclusion;
  } else {
    vec3 pos = texelFetch(waterCameraPosition, texcoord, 0).rgb;
    vec3 normal = texelFetch(waterCameraNormal, texcoord, 0).rgb;  
    vec3 direction = normalize(reflect(pos, normalize(normal)));
    vec2 hitPixel;
    vec3 hitPoint;
    ivec2 coord = ivec2(gl_FragCoord.xy);
    int frameIdx = frame % 4;
    int xIndex = (coord.x) & 3;
    int yIndex = (coord.y) & 3;
    float jitter = (xIndex * 0.0625) + (yIndex * 0.25);
    bool hit = traceScreenSpaceRay(
      pos, direction, pixelProjection, mainDepth, 0.01, 1, 32.0, jitter, 32.0, 1000.0,
      hitPixel, hitPoint);
    if (hit) {
      ivec2 hitCoord = ivec2(floor(hitPixel));
      color = texelFetch(mainColor, hitCoord, 0);
    } else {
      color = texelFetch(waterColor, texcoord, 0);
    }
    color = mix(fragMainColor, color, 0.6);
  }
  float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0)
    brightColor = vec4(color.rgb, 1.0);
  else
    brightColor = vec4(0.0, 0.0, 0.0, 1.0); 
}
