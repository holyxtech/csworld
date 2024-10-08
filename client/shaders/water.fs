#version 460 core

#include <shadow.glsl>

in VS_OUT {
    vec3 worldPos;
    vec3 worldNormal;
    vec4 cameraPos;
    vec4 cameraNormal;
} fs_in;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gNormal;

uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform vec3 cameraWorldPosition;
uniform float time;

const float normalMapWidth = 4;
const float normalMapHeight = 4;
const vec2 normalMap1Scroll = vec2(0.25,0.25) / vec2(6);
const vec2 normalMap2Scroll = vec2(-0.25,-0.25) / vec2(6);
const float specularStrength = 0.1;

void main() {
  gPosition = fs_in.cameraPos;
  gNormal = vec4(fs_in.cameraNormal.xyz, 1.f);

  vec3 fragWorldPosition = fs_in.worldPos;
  float xOffset = mod(fragWorldPosition.x + time * normalMap1Scroll.x, normalMapWidth) / normalMapWidth;
  float zOffset = mod(fragWorldPosition.z + time * normalMap1Scroll.y, normalMapHeight) / normalMapHeight;
  vec3 normal1 = texture(normalMap1, vec2(xOffset, zOffset)).rgb * 2 - 1;
  xOffset = mod(fragWorldPosition.x + time * normalMap2Scroll.x, normalMapWidth) / normalMapWidth;
  zOffset = mod(fragWorldPosition.z + time * normalMap2Scroll.y, normalMapHeight) / normalMapHeight;
  vec3 normal2 = texture(normalMap2, vec2(xOffset, zOffset)).rgb * 2 - 1;
  vec3 normal = normalize(normal1 + normal2);

  // err...
  vec3 lightDir = normalize(vec3(0.f,1.f,0.f));
  vec3 lightColor = vec3(1.f,1.f,1.f);
  color = vec4(.1f,0.3f,0.6,1.f);

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
