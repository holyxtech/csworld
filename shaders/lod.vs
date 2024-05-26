#version 460 core

layout (location = 0) in uint data;
layout (binding = 2, std430) readonly buffer ssbo1 {
    float chunkPosX[];
};
layout (binding = 3, std430) readonly buffer ssbo2 {
    float chunkPosY[];
};
layout (binding = 4, std430) readonly buffer ssbo3 {
    float chunkPosZ[];
};

uniform mat4 uTransform;

void main() {

}