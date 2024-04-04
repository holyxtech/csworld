#version 460 core

uniform sampler2DArray textureArray;

in vec2 fragUvs;
flat in float fragLayer;
flat in float fragLighting;

out vec4 color;

void main() {
    vec4 textureColor = texture(textureArray, vec3(fragUvs, fragLayer));
    vec3 finalColor = textureColor.rgb * fragLighting;
    color = vec4(finalColor, textureColor.a);
        
    if (color.a == 0.0)
      discard;
}