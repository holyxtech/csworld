#ifndef UIGRAPHICS_H
#define UIGRAPHICS_H

#include "ui.h"
#define GLM_FORCE_LEFT_HANDED
#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
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

    num_ui_textures
  };

  GLuint shader_;
  GLuint vao_;
  GLuint vbo_;
  std::vector<Vertex> mesh_;
  static constexpr int num_icons = 3;
  std::array<glm::vec2, num_icons> icon_positions_;
  glm::vec2 border_box_offset_;
};

#endif