#version 460 core

const vec2 ScreenQuadVertexCoord[6]={
     vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.f, 1.f), vec2(1.f, -1.f) };

layout (location = 0) in vec2 position;

out vec2 texCoords;

void main() {
    //texCoords = (position + vec2(1.f)) / 2;
    texCoords = (ScreenQuadVertexCoord[gl_VertexID] + vec2(1.f)) / 2;
    gl_Position = vec4(ScreenQuadVertexCoord[gl_VertexID].x, ScreenQuadVertexCoord[gl_VertexID].y,0.0, 1.0);
//    gl_Position = vec4(position, 0.0, 1.0);
}

/*  #version 460 core

layout (location = 0) in vec2 position;

out vec2 texCoords;

void main() {
    texCoords = (position + vec2(1.f)) / 2;
    gl_Position = vec4(position, 0.0, 1.0);
} 
#version 460 core

//const vec2 ScreenQuadVertexCoord[4]={ vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0) };
const vec2 ScreenQuadVertexCoord[6]={
     vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.f, 1.f), vec2(1.f, -1.f) };
//const vec2 ScreenQuadTexCoord[4]={ vec2(0, 0), vec2(1, 0), vec2(0, 1.0), vec2(1.0, 1.0) };
layout (location = 0) in vec2 position;
layout(location = 0) out vec2 TexCoords;

void main()
{
     gl_Position = vec4(ScreenQuadVertexCoord[gl_VertexID].x, ScreenQuadVertexCoord[gl_VertexID].y, 1.0, 1.0);
    TexCoord = (ScreenQuadVertexCoord[gl_VertexID] + vec2(1.f)) / 2; 
        TexCoords = (position + vec2(1.f)) / 2;
    gl_Position = vec4(position, 0.0, 1.0);
} */