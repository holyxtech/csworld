#include "ui_graphics.h"
#include <vector>
#include "render_utils.h"
#include "renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

UIGraphics::UIGraphics() {

  RenderUtils::create_shader(&shader_, "shaders/ui.vs", "shaders/ui.fs");

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBindVertexArray(vao_);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvs));
  glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, layer));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glGenTextures(1, &icon_texture_array_);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D_ARRAY, icon_texture_array_);
  GLuint num_layers = static_cast<GLuint>(num_ui_textures);
  GLsizei width = 16;
  GLsizei height = 16;
  GLsizei num_mipmaps = 1;
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, num_mipmaps, GL_RGBA8, width, height, num_layers);
  stbi_set_flip_vertically_on_load(1);
  GLsizei channels;
  std::vector<std::pair<std::string, UITexture>> textures = {
    std::make_pair("white", UITexture::white),
    std::make_pair("black", UITexture::black),
    std::make_pair("dirt", UITexture::dirt),
    std::make_pair("stone", UITexture::stone),
    std::make_pair("sandstone", UITexture::sandstone)};
  for (auto [filename, texture] : textures) {
    std::string path = "images/" + filename + ".png";
    auto* image_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(texture), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
  }
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glUseProgram(shader_);

  float w = 0.3;
  float h = 0.07;
  float b = 0.03;
  auto top_left = glm::vec2(-w, 2 * (h + b) - 1);

  int layer = static_cast<int>(UITexture::white);
  mesh_.emplace_back(Vertex{top_left, QuadCoord::tl, layer});
  mesh_.emplace_back(Vertex{glm::vec2(w, 2 * (h + b) - 1), QuadCoord::tr, layer});
  mesh_.emplace_back(Vertex{glm::vec2(w, 2 * b - 1), QuadCoord::br, layer});
  mesh_.emplace_back(Vertex{glm::vec2(-w, 2 * (h + b) - 1), QuadCoord::tl, layer});
  mesh_.emplace_back(Vertex{glm::vec2(w, 2 * b - 1), QuadCoord::br, layer});
  mesh_.emplace_back(Vertex{glm::vec2(-w, 2 * b - 1), QuadCoord::bl, layer});

  auto wt = [w](float nw) { return 2 * w * nw; };
  auto ht = [h](float nh) { return 2 * h * nh; };

  // create triangles for icons
  std::array<UITexture, num_icons> layers = {UITexture::dirt, UITexture::stone, UITexture::sandstone};

  w = 0.02; // horizontal spacing... change later
  h = 0.8;  // proportion of vertical height the icon takes up, range [0,1]
  float hs = wt(w);
  float rh = ht(1) * h;
  float rw = rh * (Renderer::window_height / static_cast<float>(Renderer::window_width));
  for (int i = 0; i < num_icons; ++i) {
    auto layer_as_ui_texture = layers[i];
    auto icon_left = top_left + glm::vec2(hs, (h - 1) / 2 * ht(1));
    icon_positions_[i] = icon_left;
    layer = static_cast<int>(layer_as_ui_texture);
    mesh_.emplace_back(Vertex{icon_left, QuadCoord::tl, layer});
    mesh_.emplace_back(Vertex{icon_left + glm::vec2(rw, 0), QuadCoord::tr, layer});
    mesh_.emplace_back(Vertex{icon_left + glm::vec2(rw, -rh), QuadCoord::br, layer});
    mesh_.emplace_back(Vertex{icon_left, QuadCoord::tl, layer});
    mesh_.emplace_back(Vertex{icon_left + glm::vec2(rw, -rh), QuadCoord::br, layer});
    mesh_.emplace_back(Vertex{icon_left + glm::vec2(0, -rh), QuadCoord::bl, layer});

    hs += rw + wt(w);
  }

  float border_offset = 0.006;
  border_box_offset_ = glm::vec2(-border_offset * (Renderer::window_height / static_cast<float>(Renderer::window_width)), border_offset);
  auto border_box_size_loc = glGetUniformLocation(shader_, "uBorderBoxSize");
  auto border_box_size = glm::vec2(rw, rh) + glm::vec2(2 * border_offset * (Renderer::window_height / static_cast<float>(Renderer::window_width)), 2 * border_offset);
  glUniform2fv(border_box_size_loc, 1, glm::value_ptr(border_box_size));

  auto icon_size_loc = glGetUniformLocation(shader_, "uIconSize");
  auto icon_size = glm::vec2(rw, rh);
  glUniform2fv(icon_size_loc, 1, glm::value_ptr(icon_size));

  // draw square in center of screen
  {
    float aim_box_size = 0.015;
    float w = Renderer::normalize_x(aim_box_size);
    float h = aim_box_size;
    auto top_left = glm::vec2(-w / 2, h / 2);
    layer = static_cast<int>(UITexture::black);
    mesh_.emplace_back(Vertex{top_left, QuadCoord::tl, layer});
    mesh_.emplace_back(Vertex{top_left + glm::vec2(w, 0), QuadCoord::tr, layer});
    mesh_.emplace_back(Vertex{top_left + glm::vec2(w, -h), QuadCoord::br, layer});
    mesh_.emplace_back(Vertex{top_left, QuadCoord::tl, layer});
    mesh_.emplace_back(Vertex{top_left + glm::vec2(w, -h), QuadCoord::br, layer});
    mesh_.emplace_back(Vertex{top_left + glm::vec2(0, -h), QuadCoord::bl, layer});
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh_.size(), mesh_.data(), GL_STATIC_DRAW);
}

void UIGraphics::render(const Renderer& renderer) const {
  glDisable(GL_DEPTH_TEST);
  glUseProgram(shader_);
  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLES, 0, mesh_.size());
  glEnable(GL_DEPTH_TEST);
}

void UIGraphics::consume_ui(UI& ui) {

  auto& diffs = ui.get_diffs();
  for (auto& diff : diffs) {
    if (diff.kind == UI::Diff::action_bar_selection) {
      auto& data = std::any_cast<const UI::Diff::ActionBarData&>(diff.data);
      auto index = data.index;
      glUseProgram(shader_);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D_ARRAY, icon_texture_array_);
      GLint texture_loc = glGetUniformLocation(shader_, "textureArray");
      glUniform1i(texture_loc, 1);
      auto border_box_top_left_loc = glGetUniformLocation(shader_, "uBorderBoxTopLeft");
      glm::vec2 pos = icon_positions_[index] + border_box_offset_;
      glUniform2fv(border_box_top_left_loc, 1, glm::value_ptr(pos));
      auto icon_top_left_loc = glGetUniformLocation(shader_, "uIconTopLeft");
      pos = icon_positions_[index];
      glUniform2fv(icon_top_left_loc, 1, glm::value_ptr(pos));

    } else if (diff.kind == UI::Diff::inventory_selection) {
      // blah blah
    }
  }

  ui.clear_diffs();
}