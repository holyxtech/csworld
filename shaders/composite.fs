#version 460 core
#extension GL_ARB_shading_language_include : require

#include <ssr.glsl>

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

uniform sampler2D mainColor;
uniform sampler2D mainDepth;

uniform sampler2D waterColor;
uniform sampler2D waterDepth;
uniform sampler2D waterPosition;
uniform sampler2D waterNormal;

uniform mat4 uPixelProjection;

void main() {
  ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
  vec4 main_depth = texelFetch(mainDepth, texcoord, 0);
  vec4 water_depth = texelFetch(waterDepth, texcoord, 0);
  vec4 main_color = texelFetch(mainColor, texcoord, 0);
 
  if (main_depth.r <= water_depth.r) {
    color = texelFetch(mainColor, texcoord, 0);
  } else {
    vec4 water_color = texelFetch(waterColor, texcoord, 0);
    vec3 waterPosition = texelFetch(waterPosition, texcoord, 0).rgb;
    vec3 waterNormal = texelFetch(waterNormal, texcoord, 0).rgb;  
  
    vec3 direction = normalize(reflect(waterPosition, normalize(waterNormal)));

    vec2 hitPixel;
    vec3 hitPoint;
    bool hit = traceScreenSpaceRay(
      waterPosition, direction, uPixelProjection, mainDepth, 0.01, 1, 10.0, 0.5, 100.0, 3000.0,
      hitPixel, hitPoint);
    
    if (hit) {
      ivec2 hitCoord = ivec2(floor(hitPixel));
      color = texelFetch(mainColor, hitCoord, 0);
    } else {
      color = water_color;
    }
    color = mix(main_color, color, 0.4);
    
  }

  float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0)
    brightColor = vec4(color.rgb, 1.0);
  else
    brightColor = vec4(0.0, 0.0, 0.0, 1.0); 
}

