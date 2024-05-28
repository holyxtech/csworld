#ifndef UIGRAPHICS_H
#define UIGRAPHICS_H

#include "ui.h"
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

class Renderer;

class UIGraphics {
public:
  UIGraphics();
  void render(const Renderer& renderer) const;
  void consume_ui(UI& ui);

private:
  struct Vertex {
    glm::vec2 position;
    glm::vec2 uvs;
    int layer;
  };
  enum UITexture {
    white,
    black,

    dirt,
    stone,
    sandstone,
    water,

    num_ui_textures
  };

  GLuint shader_;
  GLuint vao_;
  GLuint vbo_;
  GLuint icon_texture_array_;
  std::vector<Vertex> mesh_;
  std::vector<glm::vec2> icon_positions_;
  glm::vec2 border_box_offset_;
};

#endif