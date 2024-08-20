#version 460 core

const vec2 ScreenQuadVertexCoord[6]={
     vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.f, 1.f), vec2(1.f, -1.f) };

void main() {
    gl_Position = vec4(ScreenQuadVertexCoord[gl_VertexID].x, ScreenQuadVertexCoord[gl_VertexID].y,0.0, 1.0);
}