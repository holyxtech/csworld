#version 460 core

uniform sampler2DArray textureArray;
uniform vec3 sunDir;
uniform vec3 sunCol;
uniform vec3 ambientCol;

in vec2 fragUvs;
flat in float fragLayer;
flat in float fragLighting;

out vec4 color;

void main() {
/*     float diffuseFactor = max(dot(fragNormal, normalize(sunDir)), 0.0);
    vec3 diffuseColor = sunCol * diffuseFactor; */
    vec4 textureColor = texture(textureArray, vec3(fragUvs, fragLayer));
    vec3 finalColor = textureColor.rgb * fragLighting;// * (ambientCol + diffuseColor);
    color = vec4(finalColor, textureColor.a);

        
    if (color.a == 0.0)
      discard;

}