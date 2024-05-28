#version 460 core

layout (location = 0) in uint data;
layout (location = 1) in float lighting;
layout (binding = 2, std430) readonly buffer ssbo {
    vec3 chunkPos[];
};

uniform mat4 uTransform;

out vec2 fragUvs;
out vec3 fragWorldPosition;
flat out uint fragTextureId;
out float fragLighting;

const uint xpos_mask = 0x0000001F;
const uint ypos_mask = 0x000003E0;
const uint zpos_mask = 0x00007C00;
const uint normal_mask = 0x00038000;
const uint uvs_mask = 0x000C0000;
const uint texture_mask = 0xFFF00000;
const vec2 uvs[4] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
};

void main() {
    vec3 pos;
    vec3 loc = chunkPos[gl_DrawID];
    pos.x = (data & xpos_mask) + loc.x;
    pos.y = ((data & ypos_mask) >> 5) + loc.y;
    pos.z = ((data & zpos_mask) >> 10) + loc.z;

    gl_Position = uTransform * vec4(pos,1.f);    
    
    int normal = int((data & normal_mask) >> 15);

    int uvsId = int((data & uvs_mask) >> 18);

    uint textureId = uint((data & texture_mask) >> 20);

    fragTextureId = textureId;
    fragUvs = uvs[uvsId];
    fragLighting = lighting;
    fragWorldPosition = pos;
}