#version 460 core

uniform samplerCube skybox;
uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform vec3 cameraWorldPosition;
uniform float time;

in vec3 fragWorldPosition;

out vec4 color;

void main() {    
    color = vec4(0.f,0.2,0.8,1.f);
}