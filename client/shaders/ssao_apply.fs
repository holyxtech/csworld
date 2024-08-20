#version 460 core

uniform sampler2D MainColor;
uniform sampler2D SSAO;

out vec4 color;

void main() {
    ivec2 texCoord = ivec2(floor(gl_FragCoord.xy));
    vec4 tsColor = texelFetch(MainColor, texCoord, 0);
    float ambientOcclusion = texelFetch(SSAO, texCoord, 0).r;
    color = tsColor * ambientOcclusion;
}