#ifndef SKY_H
#define SKY_H

#define GLM_FORCE_LEFT_HANDED
#include <vector>
#include <GL/glew.h>


class Renderer;

class Sky {
public:
  Sky();
  void render(const Renderer& renderer) const;

private:
  GLuint shader_;
  GLuint vao_;
  GLuint vbo_;
  GLuint cube_texture_;
};

#endif