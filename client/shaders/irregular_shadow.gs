#version 460 core

layout(triangles, invocations = 3) in;
layout(triangle_strip, max_vertices = 3) out;
    
layout (std140, binding = 0) uniform LightSpaceMatrices {
  mat4 lightSpaceMatrices[3];
};

in vec2 TexCoord[];
flat in uint TextureId[];
out vec2 GeoTexCoord;
flat out uint GeoTextureId;
    
void main() {          
  for (int i = 0; i < 3; ++i) {
    gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
    GeoTexCoord = TexCoord[i];
    GeoTextureId = TextureId[i];
    gl_Layer = gl_InvocationID;
    EmitVertex();
  }
  EndPrimitive();
}