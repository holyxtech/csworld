#version 460 core

uniform samplerCube skybox;
uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform vec3 cameraWorldPosition;
uniform float time;

in vec3 fragWorldPosition;

out vec4 color;

const float normalMapWidth = 8;
const float normalMapHeight = 8;
const vec2 normalMap1Scroll = vec2(0.2,0.25);
const vec2 normalMap2Scroll = vec2(-0.25,-0.2);

const float specularStrength = 0.5;


void main() {    
    color = vec4(0.f,0.0,0.8,1.f);

    float xOffset = mod(fragWorldPosition.x + time * normalMap1Scroll.x, normalMapWidth) / normalMapWidth;
    float zOffset = mod(fragWorldPosition.z + time * normalMap1Scroll.y, normalMapHeight) / normalMapHeight;
    vec3 normal1 = texture(normalMap1, vec2(xOffset, zOffset)).rgb * 2 - 1;
    xOffset = mod(fragWorldPosition.x + time * normalMap2Scroll.x, normalMapWidth) / normalMapWidth;
    zOffset = mod(fragWorldPosition.z + time * normalMap2Scroll.y, normalMapHeight) / normalMapHeight;
    vec3 normal2 = texture(normalMap2, vec2(xOffset, zOffset)).rgb * 2 - 1;
    vec3 normal = normalize(normal1 + normal2);

    vec3 lightDir = normalize(vec3(0.f,1.f,0.f));
    vec3 lightColor = vec3(1.f,1.f,1.f);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    color.rgb += diffuse;

    // specular
    vec3 viewDir = normalize(cameraWorldPosition - fragWorldPosition);
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  

    color.rgb += specular;

}