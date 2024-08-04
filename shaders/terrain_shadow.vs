#version 460 core

layout (location = 0) in uint data;
layout (binding = 1, std430) readonly buffer ssbo {
    vec3 chunkPos[];
};

const uint xpos_mask = 0x0000003F;
const uint ypos_mask = 0x00000FC0;
const uint zpos_mask = 0x0003F000;

void main() {
    vec3 pos;
    vec3 loc = chunkPos[gl_DrawID];
    pos.x = (data & xpos_mask) + loc.x;
    pos.y = ((data & ypos_mask) >> 6) + loc.y;
    pos.z = ((data & zpos_mask) >> 12) + loc.z;
    gl_Position = vec4(pos,1.f);    
}