#version 460 core

uniform sampler2DArray textureArray;
uniform vec2 uBorderBoxSize;
uniform vec2 uBorderBoxTopLeft;
uniform vec2 uIconSize;
uniform vec2 uIconTopLeft;

in vec2 fragPosition;
in vec2 fragUvs;
flat in int fragLayer;

out vec4 color;

void main() {
    vec4 textureColor = texture(textureArray, vec3(fragUvs, fragLayer));
    if (fragPosition.x >= uBorderBoxTopLeft.x &&
        fragPosition.x <= uBorderBoxTopLeft.x + uBorderBoxSize.x &&
        fragPosition.y <= uBorderBoxTopLeft.y &&
        fragPosition.y >= uBorderBoxTopLeft.y - uBorderBoxSize.y &&

        ((fragPosition.x <= uIconTopLeft.x ||
        fragPosition.x >= uIconTopLeft.x + uIconSize.x)  ||
        (fragPosition.y >= uIconTopLeft.y  ||
        fragPosition.y <= uIconTopLeft.y - uIconSize.y))) {
        color = vec4(1.f,0.843f,0.f,1.f);
    } else {
        color = textureColor;
    }
}