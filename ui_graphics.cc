#include "ui_graphics.h"
#include <vector>
#include "input.h"
#include "render_utils.h"
#include "renderer.h"
#define NK_GLFW_GL4_IMPLEMENTATION
#include "nuklear_glfw_gl4.h"
#include "options.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

UIGraphics::UIGraphics(GLFWwindow* window, const UI& ui) : ui_(ui) {
  std::uint32_t tex_id = 0;
  ui_textures.insert({"white", tex_id++});
  ui_textures.insert({"black", tex_id++});
  for (auto [item_name, item_id] : ItemUtils::items) {
    ui_textures.insert({item_name, tex_id});
    item_to_texture_.insert({item_id, tex_id});
    tex_id++;
  }

  int max_vertex_buffer = 512 * 1024;
  int max_element_buffer = 128 * 1024;
  ctx_ = nk_glfw3_init(window, NK_GLFW3_DEFAULT, max_vertex_buffer, max_element_buffer);
  struct nk_font_atlas* atlas;
  nk_glfw3_font_stash_begin(&atlas);

  // auto* font = nk_ font_atlas_add_default(atlas, 16.f, NULL);

  nk_glfw3_font_stash_end();
  // nk_style_set_font(ctx_, &font->handle);

  struct nk_color table[NK_COLOR_COUNT];
  table[NK_COLOR_WINDOW] = nk_rgba(0, 0, 0, 0);
  table[NK_COLOR_SCROLLBAR] = nk_rgba(180, 180, 180, 255);
  table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(140, 140, 140, 255);
  nk_style_from_table(ctx_, table);
  ctx_->style.button.border = 0.f;
  ctx_->style.button.padding = {button_padding, button_padding};
  ctx_->style.button.rounding = 0.f;
  ctx_->style.window.border = 0.f;

  font_height_ = ctx_->style.font->height;
  GLuint num_layers = static_cast<GLuint>(ui_textures.size());
  GLsizei texture_width = 64;
  GLsizei texture_height = 64;
  GLsizei num_mipmaps = 1;

  for (auto [texture_name, texture_id] : ui_textures) {
    std::string path = Options::instance()->getImagePath(texture_name + ".png");
    int width, height, channels;
    auto* image_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (width != texture_width || height != texture_height) {
      auto* resized_data = new unsigned char[texture_width * texture_height * channels];
      stbir_resize_uint8_generic(
        image_data, width, height, 0,
        resized_data, texture_width, texture_height, 0,
        channels, STBIR_ALPHA_CHANNEL_NONE, 0,
        STBIR_EDGE_CLAMP,
        STBIR_FILTER_BOX,
        STBIR_COLORSPACE_SRGB,
        NULL);
      stbi_image_free(image_data);
      image_data = resized_data;
    }

    int tex_index = nk_glfw3_create_texture(image_data, texture_width, texture_height);
    auto img = nk_image_id(tex_index);
    icons_.insert({texture_id, std::move(img)});

    stbi_image_free(image_data);
  }

  glfwSetScrollCallback(window, nk_gflw3_scroll_callback);
}

void UIGraphics::render() {
  nk_glfw3_new_frame();
  ctx_->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
  ctx_->style.window.scrollbar_size = {0.f, 0.f};
  ctx_->style.window.padding = {0.f, 0.f};
  ctx_->style.window.spacing = {0.f, 0.f};
  if (nk_begin(ctx_, "nuklear window", nk_rect(0, 0, Renderer::window_width, Renderer::window_height), 0)) {
    nk_layout_set_min_row_height(ctx_, 0);

    auto* canvas = nk_window_get_canvas(ctx_);
    nk_layout_space_begin(ctx_, NK_STATIC, 0, 1);

    int action_bar_height = Renderer::window_height * 0.07;
    int action_bar_width = (action_bar_height + 2 * button_padding) * UI::action_bar_size; // doesn't account for widget_spacing
    int width_offset = Renderer::window_width / 2 - action_bar_width / 2;
    int height_offset = (Renderer::window_height)-action_bar_height - 2 * widget_spacing;
    auto action_bar_rect = nk_rect(width_offset, height_offset, action_bar_width, Renderer::window_height);
    nk_layout_space_push(ctx_, action_bar_rect);

    ctx_->style.window.spacing = {widget_spacing, widget_spacing};
    if (nk_group_begin(ctx_, "action_bar", 0)) {
      nk_layout_row_dynamic(ctx_, action_bar_height, UI::action_bar_size);
      auto& action_bar = ui_.get_action_bar();
      std::size_t idx = 0;
      auto active_index = ui_.get_active_index();
      UITexture texture;
      for (; idx < active_index; ++idx) {
        auto item = action_bar[idx];
        if (item.has_value())
          texture = item_to_texture_.at(item.value());
        else
          texture = ui_textures.at("white");
        nk_button_image(ctx_, icons_.at(texture));
      }
      nk_style_push_style_item(ctx_, &ctx_->style.button.normal, nk_style_item_color(nk_rgb(255, 215, 0)));
      auto item = action_bar[idx++];
      if (item.has_value())
        texture = item_to_texture_.at(item.value());
      else
        texture = ui_textures.at("white");
      nk_button_image(ctx_, icons_.at(texture));
      nk_style_pop_style_item(ctx_);
      for (; idx < action_bar.size(); ++idx) {
        auto item = action_bar[idx];
        if (item.has_value())
          texture = item_to_texture_.at(item.value());
        else
          texture = ui_textures.at("white");
        nk_button_image(ctx_, icons_.at(texture));
      }
      nk_group_end(ctx_);
    }

    {
      int crosshair_width = 10;
      int crosshair_height = 10;
      int width_offset = Renderer::window_width / 2 - crosshair_width / 2;
      int height_offset = Renderer::window_height / 2 - crosshair_width / 2;
      auto crosshair_rect = nk_rect(width_offset, height_offset, crosshair_width, crosshair_height);

      nk_layout_space_push(ctx_, crosshair_rect);

      nk_style_push_vec2(ctx_, &ctx_->style.button.padding, nk_vec2(0, 0));
      nk_button_image(ctx_, icons_.at(ui_textures.at("black")));
      nk_style_pop_vec2(ctx_);
    }

    nk_layout_space_end(ctx_);
    nk_layout_reset_min_row_height(ctx_);
  }
  nk_end(ctx_);

  if (ui_.is_inv_open()) {
    ctx_->style.window.fixed_background = nk_style_item_color(nk_rgba(194, 185, 184, 255));
    ctx_->style.window.spacing = {widget_spacing, widget_spacing};
    ctx_->style.window.padding = {4.f, 4.f};
    float scrollbar_width = 20.f;
    ctx_->style.window.scrollbar_size = {scrollbar_width, 0.f};
    int inv_height = Renderer::window_height * 0.5;
    int inv_width = inv_height;
    int width_offset = (Renderer::window_width - inv_width) / 2;
    int height_offset = (Renderer::window_height - inv_height) / 2;
    if (nk_begin(ctx_, "inv window", nk_rect(width_offset, height_offset, inv_width, inv_height), NK_WINDOW_BORDER)) {
      int num_cols = 6;
      int icon_size = (inv_width - (num_cols) * (widget_spacing)-scrollbar_width) / num_cols - 0;

      nk_layout_row_static(ctx_, icon_size, icon_size, num_cols);
      auto& inv = ui_.get_inv();
      bool icon_is_hovered = false;

      for (auto item : inv) {
        nk_style_push_style_item(ctx_, &ctx_->style.button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx_, &ctx_->style.button.active, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx_, &ctx_->style.button.hover, nk_style_item_color(nk_rgba(255, 215, 0, 255)));
        auto texture = item_to_texture_.at(item);
        nk_button_image(ctx_, icons_.at(texture));
        if (ctx_->last_widget_state & NK_WIDGET_STATE_HOVER) {
          hovering_ = item;
          icon_is_hovered = true;
        }
        nk_style_pop_style_item(ctx_);
        nk_style_pop_style_item(ctx_);
        nk_style_pop_style_item(ctx_);
      }

      if (!icon_is_hovered)
        hovering_ = {};
    }
    nk_end(ctx_);
  }

  nk_glfw3_render(NK_ANTI_ALIASING_ON);
}

std::optional<Item> UIGraphics::get_hovering() const {
  return hovering_;
}