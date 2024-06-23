#include "ui_graphics.h"
#include <vector>
#include "render_utils.h"
#include "renderer.h"
#define NK_GLFW_GL4_IMPLEMENTATION
#include "nuklear_glfw_gl4.h"
#include "options.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

UIGraphics::UIGraphics(GLFWwindow* window, const UI& ui) : ui_(ui) {
  std::initializer_list<std::pair<const Item, UITexture>> init_list = {
    {Item::empty, UITexture::white},
    {Item::dirt, UITexture::dirt},
    {Item::stone, UITexture::stone},
    {Item::sandstone, UITexture::sandstone},
    {Item::water, UITexture::water},
  };
  for (auto& pair : init_list)
    item_to_texture_.emplace(pair);

  int max_vertex_buffer = 512 * 1024;
  int max_element_buffer = 128 * 1024;
  ctx_ = nk_glfw3_init(window, NK_GLFW3_DEFAULT, max_vertex_buffer, max_element_buffer);
  struct nk_font_atlas* atlas;
  nk_glfw3_font_stash_begin(&atlas);

  // auto* font = nk_font_atlas_add_default(atlas, 16.f, NULL);

  nk_glfw3_font_stash_end();
  // nk_style_set_font(ctx_, &font->handle);

  struct nk_color table[NK_COLOR_COUNT];

  table[NK_COLOR_WINDOW] = nk_rgba(0, 0, 0, 0);
  nk_style_from_table(ctx_, table);
  ctx_->style.button.border = 0.f;
  ctx_->style.button.padding = {button_padding, button_padding};
  ctx_->style.button.rounding = 0.f;
  ctx_->style.window.spacing = {4.f, 4.f};
  ctx_->style.window.border = 0.f;
  ctx_->style.window.padding = {0.f, 0.f};
  ctx_->style.window.scrollbar_size = {0.f, 0.f};

  font_height_ = ctx_->style.font->height;
  GLuint num_layers = static_cast<GLuint>(num_ui_textures);
  GLsizei width = 16;
  GLsizei height = 16;
  GLsizei channels;
  GLsizei num_mipmaps = 1;
  std::vector<std::pair<std::string, UITexture>> textures = {
    std::make_pair("white", UITexture::white),
    std::make_pair("black", UITexture::black),
    std::make_pair("dirt", UITexture::dirt),
    std::make_pair("stone", UITexture::stone),
    std::make_pair("sandstone", UITexture::sandstone),
    std::make_pair("water", UITexture::water)};

  for (auto [filename, texture] : textures) {
    std::string path = Options::instance()->getImagePath(filename + ".png");
    auto* image_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    int tex_index = nk_glfw3_create_texture(image_data, width, height);
    auto img = nk_image_id(tex_index);
    icons_.insert({texture, std::move(img)});

    stbi_image_free(image_data);
  }
}

void UIGraphics::render() const {
  nk_glfw3_new_frame();
  if (nk_begin(ctx_, "nuklear window", nk_rect(0, 0, Renderer::window_width, Renderer::window_height), 0)) {
    nk_layout_set_min_row_height(ctx_, 0);

    auto* canvas = nk_window_get_canvas(ctx_);
    nk_layout_space_begin(ctx_, NK_STATIC, 0, 1);

    int action_bar_height = Renderer::window_height * 0.07;
    int action_bar_width = (action_bar_height + 2 * button_padding) * UI::action_bar_size; // doesn't account for widget_spacing
    int width_offset = Renderer::window_width / 2 - action_bar_width / 2;
    int height_offset = (Renderer::window_height * 0.99) - action_bar_height;
    auto action_bar_rect = nk_rect(width_offset, height_offset, action_bar_width, Renderer::window_height);
    nk_layout_space_push(ctx_, action_bar_rect);

    ctx_->style.window.spacing = {widget_spacing, widget_spacing};
    if (nk_group_begin(ctx_, "action_bar", 0)) {
      nk_layout_row_dynamic(ctx_, action_bar_height, UI::action_bar_size);
      auto& action_bar = ui_.get_action_bar();
      std::size_t idx = 0;
      auto active_index = ui_.get_active_index();
      for (; idx < active_index; ++idx) {
        auto texture = item_to_texture_.at(action_bar[idx]);
        nk_button_image(ctx_, icons_.at(texture));
      }
      nk_style_push_style_item(ctx_, &ctx_->style.button.normal, nk_style_item_color(nk_rgb(255, 215, 0)));
      auto texture = item_to_texture_.at(action_bar[idx++]);
      nk_button_image(ctx_, icons_.at(texture));
      nk_style_pop_style_item(ctx_);
      for (; idx < action_bar.size(); ++idx) {
        auto texture = item_to_texture_.at(action_bar[idx]);
        nk_button_image(ctx_, icons_.at(texture));
      }
      nk_group_end(ctx_);
    }

    {
      ctx_->style.window.spacing = {0.f, 0.f};
      int crosshair_width = 10;
      int crosshair_height = 10;
      int width_offset = Renderer::window_width / 2 - crosshair_width / 2;
      int height_offset = Renderer::window_height / 2 - crosshair_width / 2;
      auto crosshair_rect = nk_rect(width_offset, height_offset, crosshair_width, crosshair_height);

      nk_layout_space_push(ctx_, crosshair_rect);

      auto screen_rect = nk_layout_space_rect_to_screen(ctx_, crosshair_rect);
      nk_style_push_vec2(ctx_, &ctx_->style.button.padding, nk_vec2(0, 0));
      nk_button_image(ctx_, icons_.at(UITexture::black));
      nk_style_pop_vec2(ctx_);
    }

    nk_layout_space_end(ctx_);
    nk_layout_reset_min_row_height(ctx_);
  }
  nk_end(ctx_);
  nk_glfw3_render(NK_ANTI_ALIASING_ON);
}