#version 460 core

uniform sampler2DArray textureArray;

in vec2 fragUvs;;
flat in float fragLayer;

out vec4 color;

void main() {
//    color = vec4(0.8f, 0.5f, 0.2f, 1.0f);
    color = texture(textureArray, vec3(fragUvs, fragLayer));
}