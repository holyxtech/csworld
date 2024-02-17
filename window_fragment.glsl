#version 460 core

uniform sampler2D MainColor;
uniform sampler2D WaterColor;
uniform sampler2D MainDepth;
uniform sampler2D WaterDepth;

out vec4 color;

void main() {
    ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
    vec4 main_depth = texelFetch(MainDepth, texcoord, 0);
    vec4 water_depth = texelFetch(WaterDepth, texcoord, 0);

    if (main_depth.r < water_depth.r) 
        color = texelFetch(MainColor, texcoord, 0);
    else 
        color = mix(texelFetch(MainColor, texcoord, 0), texelFetch(WaterColor, texcoord, 0), 0.8);
}