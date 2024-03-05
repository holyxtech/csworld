#version 460 core

uniform sampler2DArray textureArray;
uniform vec3 sunDir;
uniform vec3 sunCol;
uniform vec3 ambientCol;

in vec2 fragUvs;
in vec3 fragNormal;
flat in float fragLayer;

out vec4 color;

void main() {
    float diffuseFactor = max(dot(fragNormal, normalize(sunDir)), 0.0);
    vec3 diffuseColor = sunCol * diffuseFactor;
    vec4 textureColor = texture(textureArray, vec3(fragUvs, fragLayer));
    vec3 finalColor = textureColor.rgb * (ambientCol + diffuseColor);
    color = vec4(finalColor, textureColor.a);

    /* if (color.w < 1) color = vec4(1,0,0,1);
    else color = vec4(0,1,0,1);
     */if (color.a <= 0.0) {
    discard;
}

}