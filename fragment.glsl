#version 460 core

uniform sampler2DArray textureArray;

in vec2 fragUvs;;
flat in float fragLayer;

out vec4 color;

void main() {
    color = texture(textureArray, vec3(fragUvs, fragLayer));
}