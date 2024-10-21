#version 460 core
uniform sampler2D UIColor;      
uniform int height;

out vec4 FragColor;

void main() {
    ivec2 texCoord = ivec2(floor(gl_FragCoord.x), height - 1 - floor(gl_FragCoord.y));
    vec4 col = texelFetch(UIColor, texCoord, 0);
    if (col.a == 0)
        discard;
    FragColor = col;
}