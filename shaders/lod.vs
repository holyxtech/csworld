#version 460 core

layout (location = 0) in uint data;
layout (binding = 1, std430) readonly buffer ssbo {
    vec3 chunkPos[];
};

uniform mat4 uTransform;

out vec2 fragUvs;
out vec3 fragWorldPosition;
flat out uint fragTextureId;

const uint xpos_mask = 0x0000000F;
const uint ypos_mask = 0x000001F0;
const uint zpos_mask = 0x00001E00;
const uint normal_mask = 0x0000E000;
const uint uvs_mask = 0x00030000;
const uint texture_mask = 0xFFFC0000;
const vec2 uvs[4] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
};

void main() {
    vec3 pos;
    vec3 loc = chunkPos[gl_DrawID];
    pos.x = 2*(data & xpos_mask) + loc.x;
    pos.y = ((data & ypos_mask) >> 4) + loc.y;
    pos.z = 2*((data & zpos_mask) >> 9) + loc.z;
    gl_Position = uTransform * vec4(pos,1.f);    

    int normal = int((data & normal_mask) >> 13);
    int uvsId = int((data & uvs_mask) >> 16);
    uint textureId = uint((data & texture_mask) >> 18);

    fragTextureId = textureId;
    vec2 texcoords = uvs[uvsId];
    texcoords.x /= 32;
    texcoords.y /= 32;
    fragUvs = vec2(texcoords);
}