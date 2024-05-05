#version 460 core

uniform sampler2D mainColor;
uniform sampler2D waterColor;
uniform sampler2D mainDepth;
uniform sampler2D waterDepth;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

void main() {
  ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
  vec4 main_depth = texelFetch(mainDepth, texcoord, 0);
  vec4 water_depth = texelFetch(waterDepth, texcoord, 0);
  //color = texelFetch(mainColor, texcoord, 0);
  if (main_depth.r <= water_depth.r) 
    color = texelFetch(mainColor, texcoord, 0);
  else 
    color = mix(texelFetch(mainColor, texcoord, 0), texelFetch(waterColor, texcoord, 0), 0.8); 

  float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0)
    brightColor = vec4(color.rgb, 1.0);
  else
    brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}