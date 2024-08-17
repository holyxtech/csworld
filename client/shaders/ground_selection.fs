#version 460 core

layout (binding = 0) uniform sampler2D MainDepth;
layout (binding = 1) uniform sampler2D MainColor;

out vec4 color;

void main() {
    ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
    vec4 envDepth = texelFetch(MainDepth, texcoord, 0);
    vec4 envColor = texelFetch(MainColor, texcoord, 0);
    if (envDepth.r > gl_FragCoord.z) {
        vec4 tileColor = vec4(.729f,.486f,1.f,1.f);
        color = mix(tileColor, envColor, 0.5);
    }
    else {
        discard;
    }
}