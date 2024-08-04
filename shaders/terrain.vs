#version 460 core

layout (location = 0) in uint data;
layout (binding = 1, std430) readonly buffer ssbo {
    vec3 chunkPos[];
};

out VS_OUT {
  vec3 worldPos;
  vec3 worldNormal;
} vs_out;
out vec2 fragUvs;
flat out uint fragTextureId;

uniform mat4 uTransform; // should be in UBO

const uint xpos_mask = 0x0000003F;
const uint ypos_mask = 0x00000FC0;
const uint zpos_mask = 0x0003F000;
const uint normal_mask = 0x001C0000;
const uint uvs_mask = 0x00600000;
const uint texture_mask = 0xFF800000;
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
    pos.x = (data & xpos_mask) + loc.x;
    pos.y = ((data & ypos_mask) >> 6) + loc.y;
    pos.z = ((data & zpos_mask) >> 12) + loc.z;

    gl_Position = uTransform * vec4(pos,1.f);
    
    int normalId = int((data & normal_mask) >> 18);
    int uvsId = int((data & uvs_mask) >> 21);
    uint textureId = uint((data & texture_mask) >> 23);

    fragTextureId = textureId;
    fragUvs = uvs[uvsId];
    vs_out.worldPos = pos;
    vs_out.worldNormal = normals[normalId];
}