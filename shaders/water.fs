#version 460 core
#extension GL_ARB_shading_language_include : require

#include <shadow.glsl>

in VS_OUT {
    vec3 worldPos;
    vec3 worldNormal;
} fs_in;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 gCameraPosition;
layout (location = 2) out vec4 gCameraNormal;

uniform mat4 uView; // should be in UBO
uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform vec3 cameraWorldPosition;
uniform float time;

const float normalMapWidth = 8;
const float normalMapHeight = 8;
const vec2 normalMap1Scroll = vec2(0.2,0.25);
const vec2 normalMap2Scroll = vec2(-0.25,-0.2);
const float specularStrength = 0.5;

void main() {
  gCameraPosition = uView * vec4(fs_in.worldPos, 1.f);
  vec4 cameraNormal = uView * vec4(fs_in.worldNormal, 0.f);
  gCameraNormal = vec4(cameraNormal.xyz, 1.f);

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
  color = vec4(0.f,0.0,0.8,1.f);

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