#version 460 core
#extension GL_ARB_shading_language_include : require

#include <common.glsl>

layout (location = 0) in uint data;
layout (binding = 1, std430) readonly buffer ssbo {
    vec3 chunkPos[];
};

out VS_OUT {
  vec3 worldPos;
  vec3 worldNormal;
  vec3 cameraPos;
  vec3 cameraNormal;
  vec2 uvs;
} vs_out;
flat out uint fragTextureId;

uniform mat4 uView;
uniform mat4 uTransform; // should be in UBO

const uint xposMask = 0x0000003F;
const uint yposMask = 0x00000FC0;
const uint zposMask = 0x0003F000;
const uint normalMask = 0x001C0000;
const uint uvsMask = 0x00600000;
const uint textureMask = 0xFF800000;
const vec2 uvs[4] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
};
const vec3 normals[6] = {
    vec3(-1.f,0.f,0.f),
    vec3(1.f,0.f,0.f),
    vec3(0.f,-1.f,0.f),
    vec3(0.f,1.f,0.f),
    vec3(0.f,0.f,-1.f),
    vec3(0.f,0.f,1.f)
};

void main() {
    vec3 pos;
    vec3 loc = chunkPos[gl_DrawID];
    pos.x = (data & xposMask) + loc.x;
    pos.y = ((data & yposMask) >> 6) + loc.y;
    pos.z = ((data & zposMask) >> 12) + loc.z;

    gl_Position = uTransform * vec4(pos,1.f);
    
    int normalId = int((data & normalMask) >> 18);
    int uvsId = int((data & uvsMask) >> 21);
    uint textureId = uint((data & textureMask) >> 23);

    fragTextureId = textureId;
    vs_out.uvs = uvs[uvsId];
    vs_out.worldPos = pos;
    vec3 normal = normals[normalId];
    vs_out.worldNormal = normal;
    vec4 cameraPos = uView * vec4(pos, 1.f);
    vs_out.cameraPos = cameraPos.xyz;
    vs_out.cameraNormal = (normalMatrix*vec4(normal,0.f)).xyz;
}