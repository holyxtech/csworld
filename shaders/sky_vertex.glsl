#version 460 core

uniform mat4 uTransform;

layout (location = 0) in vec3 position;

out vec3 TexCoords;

void main() {
    TexCoords = position;
    vec4 pos = uTransform * vec4(position, 1.0); 
    gl_Position = pos.xyww;

}  