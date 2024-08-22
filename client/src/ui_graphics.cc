#include "ui_graphics.h"
#include <vector>
#include "input.h"
#include "render_utils.h"
#include "renderer.h"
#include "sim.h"
#define NK_GLFW_GL4_IMPLEMENTATION
#include "nuklear_glfw_gl4.h"
#include "options.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

float UIGraphics::action_button_spacing = 0.f;
float UIGraphics::action_button_border = 8.f;
float UIGraphics::action_button_padding = 0.f;
struct nk_color UIGraphics::action_border_default_color = nk_rgb(204, 204, 204);
struct nk_color UIGraphics::action_border_selected_color = nk_rgb(81, 125, 47);
float UIGraphics::inv_button_spacing = 0.f;
float UIGraphics::inv_button_padding = 0.f;
float UIGraphics::inv_scrollbar_width = 20.f;
float UIGraphics::inv_button_border = 8.f;
struct nk_color UIGraphics::inv_border_hover_color = nk_rgb(255, 200, 30);
struct nk_color UIGraphics::inv_background_color = nk_rgb(230, 230, 230);
struct nk_color UIGraphics::build_options_background_color = nk_rgb(80, 80, 80);

UIGraphics::UIGraphics(Sim& sim) : renderer_(sim.get_renderer()), ui_(sim.get_ui()) {
  std::uint32_t tex_id = 0;
  for (auto [item_name, item_id] : ItemUtils::items) {
    ui_textures.insert({item_name, tex_id});
    item_to_texture_.insert({item_id, tex_id});
    tex_id++;
  }

  int max_vertex_buffer = 512 * 1024;
  int max_element_buffer = 128 * 1024;
  GLFWwindow* window = sim.get_window();
  ctx_ = nk_glfw3_init(window, NK_GLFW3_INSTALL_CHAR_CALLBACK, max_vertex_buffer, max_element_buffer);
  struct nk_font_atlas* atlas;
  nk_glfw3_font_stash_begin(&atlas);

  auto* font = nk_font_atlas_add_default(atlas, 24.f, NULL);
  /* auto font_path = Options::instance()->getFontPath("ProggyTiny.ttf");
  struct nk_font* font = nk_font_atlas_add_from_file(atlas, font_path.c_str(), 16, 0); */
  nk_glfw3_font_stash_end();
  nk_style_set_font(ctx_, &font->handle);

  struct nk_color table[NK_COLOR_COUNT];
  table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
  table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
  table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
  table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
  table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
  table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
  table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
  table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
  table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
  table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
  table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
  table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
  table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
  table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
  table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
  table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
  table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
  table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
  table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
  table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
  table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
  table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
  table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
  table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
  table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
  table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
  table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
  table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
  ctx_->style.slider.bar_height = 50;

  nk_style_from_table(ctx_, table);

  font_height_ = ctx_->style.font->height;
  GLuint num_layers = static_cast<GLuint>(ui_textures.size());
  GLsizei texture_width = 64;
  GLsizei texture_height = 64;
  GLsizei num_mipmaps = 1;

  for (auto [texture_name, texture_id] : ui_textures) {
    std::string path = Options::instance()->getImagePath(texture_name + ".png");
    int width, height, channels;
    auto* image_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    int num_pixels = width * height;
    for (int i = 0; i < num_pixels; ++i) {
      unsigned char* pixel = &image_data[i * channels];

      if (pixel[3] == 0) {
        pixel[0] = action_border_default_color.r;
        pixel[1] = action_border_default_color.g;
        pixel[2] = action_border_default_color.b;
        pixel[3] = 255;
      }
    }

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

void UIGraphics::render_options_window() {

  nk_style_push_float(ctx_, &ctx_->style.slider.bar_height, 50.f);
  if (nk_begin(ctx_, "Options", nk_rect(options_window_offset_.x, options_window_offset_.y, 500, 500), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
    struct nk_rect bounds = nk_window_get_bounds(ctx_);
    options_window_offset_.x = bounds.x;
    options_window_offset_.y = bounds.y;

    // Create a static row with one element: the "test" button
    /*     nk_layout_row_static(ctx_, 50, 100, 1);
        if (nk_button_label(ctx_, "test")) {
          // action_events_.enqueue(Action{Action::terrain_generation, Action::TerrainGenerationData{}});
        }
     */
    // Modify slider style to have square knobs
    struct nk_style_slider* slider_style = &ctx_->style.slider;
    slider_style->cursor_size = nk_vec2(40, 40); // Make the knob square by setting equal width and height

    const float row_size[] = {100, 300, 50};
    nk_layout_row(ctx_, NK_STATIC, 50, 3, row_size);
    auto& sky = renderer_.get_sky();

    auto calculateAzimuthPolar = [](const glm::vec3& dir) {
      float polar = glm::degrees(atan2(dir.y, dir.x)); // Azimuth angle in degrees
      float azimuth = glm::degrees(acos(dir.z));       // Polar angle in degrees

      // Ensure azimuth is within [0, 360) range

      return std::make_pair(azimuth, polar);
    };
    auto [a, p] = calculateAzimuthPolar(sky.get_sun_dir());
    static float azimuth_value = a;
    static float polar_value = p;
    // Slider for Azimuth Angle
    nk_label(ctx_, "Azimuth:", NK_TEXT_LEFT);
    nk_slider_float(ctx_, 0.0f, &azimuth_value, 360.0f, 1.0f);
    nk_labelf(ctx_, NK_TEXT_LEFT, "%.0f", azimuth_value); // Display slider value as an integer

    // Slider for Polar Angle
    nk_label(ctx_, "Polar:", NK_TEXT_LEFT);
    nk_slider_float(ctx_, 0.0f, &polar_value, 180.0f, 1.0f); // Limit range to 0-180 degrees
    nk_labelf(ctx_, NK_TEXT_LEFT, "%.0f", polar_value);      // Display slider value as an integer

    nk_layout_row_dynamic(ctx_, 30, 1);
    auto& uniform_value = renderer_.get_uniform_value("ShadowBias");

    static std::string bias_string = std::to_string(std::any_cast<float>(uniform_value.data));
    static int num_chars = bias_string.length();
    bias_string.resize(64);
    nk_label(ctx_, "Shadow Bias:", NK_TEXT_LEFT);
    nk_edit_string(ctx_, NK_EDIT_SIMPLE, bias_string.data(), &num_chars, 64, nk_filter_float);
    bias_string[num_chars] = '\0';
    float bias_value = std::strtof(bias_string.c_str(), nullptr);
    renderer_.set_uniform_value("ShadowBias", UniformValue{UniformType::Float, bias_value});
    // Update sky direction with clamped angles

    sky.set_sun_dir(azimuth_value, polar_value);
  }
  nk_style_pop_float(ctx_);
  nk_end(ctx_);
}

void UIGraphics::render_first_person_ui() {
  nk_glfw3_new_frame();

  uint32_t flags = 0;
  if (ui_.is_inv_open() || ui_.is_options_open()) {
    flags |= NK_WINDOW_NO_INPUT;
  }

  nk_style_push_float(ctx_, &ctx_->style.button.border, 0.f);
  nk_style_push_float(ctx_, &ctx_->style.button.rounding, 0.f);
  nk_style_push_float(ctx_, &ctx_->style.window.border, 0.f);
  nk_style_push_float(ctx_, &ctx_->style.button.border, action_button_border);
  nk_style_push_vec2(ctx_, &ctx_->style.window.padding, {0, 0});
  nk_style_push_vec2(ctx_, &ctx_->style.window.spacing, {0, 0});
  nk_style_push_vec2(ctx_, &ctx_->style.window.group_padding, {0, 0});
  nk_style_push_vec2(ctx_, &ctx_->style.window.scrollbar_size, {0.f, 0.f});
  nk_style_push_vec2(ctx_, &ctx_->style.button.padding, {action_button_padding, action_button_padding});
  nk_style_push_style_item(ctx_, &ctx_->style.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
  if (nk_begin(ctx_, "nuklear window", nk_rect(0, 0, Renderer::window_width, Renderer::window_height), flags)) {
    nk_layout_set_min_row_height(ctx_, 0);
    nk_layout_space_begin(ctx_, NK_STATIC, 0, 1);
    float action_bar_height = Renderer::window_height * 0.08;
    float action_image_width = action_bar_height - (2 * action_button_border);
    float action_button_width = action_bar_height;
    float spacing = (UI::action_bar_size - 1) * action_button_spacing;
    float action_bar_width = action_button_width * UI::action_bar_size + spacing;
    float width_offset = Renderer::window_width / 2 - action_bar_width / 2;
    float height_offset = (Renderer::window_height)-action_bar_height; // - action_button_spacing;
    auto action_bar_rect = nk_rect(width_offset, height_offset, action_bar_width, Renderer::window_height);
    nk_layout_space_push(ctx_, action_bar_rect);
    nk_style_push_vec2(ctx_, &ctx_->style.window.spacing, {action_button_spacing, action_button_spacing});
    if (nk_group_begin(ctx_, "action_bar", 0)) {
      nk_layout_row_static(ctx_, action_bar_height, action_button_width, UI::action_bar_size);
      nk_style_push_color(ctx_, &ctx_->style.button.border_color, action_border_default_color);
      auto& action_bar = ui_.get_action_bar();
      std::size_t idx = 0;
      auto active_index = ui_.get_active_index();
      UITexture texture;
      for (; idx < active_index; ++idx) {
        auto item = action_bar[idx];
        if (item.has_value()) {
          texture = item_to_texture_.at(item.value());
          nk_button_image(ctx_, icons_.at(texture));
        } else {
          nk_button_color(ctx_, action_border_default_color);
        }
      }
      nk_style_push_color(ctx_, &ctx_->style.button.border_color, action_border_selected_color);
      auto item = action_bar[idx++];
      if (item.has_value()) {
        texture = item_to_texture_.at(item.value());
        nk_button_image(ctx_, icons_.at(texture));
      } else {
        nk_button_color(ctx_, action_border_default_color);
      }
      nk_style_pop_color(ctx_);
      for (; idx < action_bar.size(); ++idx) {
        auto item = action_bar[idx];
        if (item.has_value()) {
          texture = item_to_texture_.at(item.value());
          nk_button_image(ctx_, icons_.at(texture));
        } else {
          nk_button_color(ctx_, action_border_default_color);
        }
      }
      nk_style_pop_color(ctx_);
      nk_group_end(ctx_);
    }
    nk_style_pop_vec2(ctx_);

    {
      int crosshair_width = 10;
      int crosshair_height = 10;
      float border_width = 0.f;
      int width_offset = Renderer::window_width / 2 - crosshair_width / 2 - border_width;
      int height_offset = Renderer::window_height / 2 - crosshair_width / 2 - border_width;
      auto crosshair_rect = nk_rect(width_offset, height_offset, crosshair_width, crosshair_height);
      nk_layout_space_push(ctx_, crosshair_rect);

      nk_style_push_vec2(ctx_, &ctx_->style.button.padding, nk_vec2(0, 0));
      nk_style_push_float(ctx_, &ctx_->style.button.border, border_width);
      nk_button_color(ctx_, nk_rgb(0, 0, 0));
      nk_style_pop_float(ctx_);
      nk_style_pop_vec2(ctx_);
    }

    nk_layout_space_end(ctx_);
    nk_layout_reset_min_row_height(ctx_);
  }
  nk_style_pop_style_item(ctx_);
  nk_style_pop_vec2(ctx_);
  nk_style_pop_vec2(ctx_);
  nk_style_pop_vec2(ctx_);
  nk_style_pop_vec2(ctx_);
  nk_style_pop_vec2(ctx_);
  nk_style_pop_float(ctx_);
  nk_style_pop_float(ctx_);
  nk_style_pop_float(ctx_);
  nk_style_pop_float(ctx_);
  nk_end(ctx_);

  if (ui_.is_inv_open()) {
    nk_style_push_style_item(ctx_, &ctx_->style.window.fixed_background, nk_style_item_color(inv_background_color));
    nk_style_push_vec2(ctx_, &ctx_->style.window.padding, {inv_button_padding, inv_button_padding});
    nk_style_push_vec2(ctx_, &ctx_->style.window.spacing, {inv_button_spacing, inv_button_spacing});
    nk_style_push_vec2(ctx_, &ctx_->style.window.scrollbar_size, {inv_scrollbar_width, 0.f});
    nk_style_push_float(ctx_, &ctx_->style.button.rounding, 0.f);
    nk_style_push_float(ctx_, &ctx_->style.window.border, 0.f);
    nk_style_push_float(ctx_, &ctx_->style.button.border, inv_button_border);
    nk_style_push_vec2(ctx_, &ctx_->style.button.padding, nk_vec2(inv_button_padding, inv_button_padding));
    int inv_height = Renderer::window_height * 0.5;
    int inv_width = inv_height;
    int width_offset = (Renderer::window_width - inv_width) / 2;
    int height_offset = (Renderer::window_height - inv_height) / 2;
    if (nk_begin(ctx_, "inv window", nk_rect(width_offset, height_offset, inv_width, inv_height), NK_WINDOW_BORDER)) {
      int num_cols = 7;
      int icon_size = (inv_width - inv_scrollbar_width) / num_cols;
      nk_layout_row_static(ctx_, icon_size, icon_size, num_cols);
      auto& inv = ui_.get_inv();
      bool icon_is_hovered = false;
      nk_style_push_color(ctx_, &ctx_->style.button.border_color, inv_background_color);
      for (auto item : inv) {
        auto texture = item_to_texture_.at(item);
        struct nk_rect bounds = nk_widget_bounds(ctx_);
        int is_hovered = nk_input_is_mouse_hovering_rect(&ctx_->input, bounds);
        if (is_hovered) {
          nk_style_push_color(ctx_, &ctx_->style.button.border_color, inv_border_hover_color);
          nk_style_push_float(ctx_, &ctx_->style.button.border, inv_button_border);
          nk_button_image(ctx_, icons_.at(texture));
          nk_style_pop_float(ctx_);
          nk_style_pop_color(ctx_);
          hovering_ = item;
          icon_is_hovered = true;
        } else {
          nk_button_image(ctx_, icons_.at(texture));
        }
      }
      nk_style_pop_color(ctx_);
      if (!icon_is_hovered)
        hovering_ = {};
    }
    nk_style_pop_vec2(ctx_);
    nk_style_pop_float(ctx_);
    nk_style_pop_float(ctx_);
    nk_style_pop_float(ctx_);
    nk_style_pop_vec2(ctx_);
    nk_style_pop_vec2(ctx_);
    nk_style_pop_vec2(ctx_);
    nk_style_pop_style_item(ctx_);
    nk_end(ctx_);
  }

  if (ui_.is_options_open()) {
    render_options_window();
  }

  nk_glfw3_render(NK_ANTI_ALIASING_ON);
}

void UIGraphics::render_build_ui() {
  nk_glfw3_new_frame();
  if (
    nk_begin(
      ctx_, "Build Options", nk_rect(0, 0, Renderer::window_width / 8, Renderer::window_height / 2),
      NK_WINDOW_MOVABLE | NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
    nk_layout_row_static(ctx_, 40, 130, 1);
    if (nk_button_label(ctx_, "Generate")) {
      action_events_.enqueue(Action{Action::terrain_generation, Action::TerrainGenerationData{}});
    }
  }

  if (nk_window_is_hovered(ctx_)) {
    mouse_captured_ = true;
  } else {
    mouse_captured_ = false;
  }

  nk_end(ctx_);

  nk_glfw3_render(NK_ANTI_ALIASING_ON);
}

std::optional<Item> UIGraphics::get_hovering() const {
  return hovering_;
}

void UIGraphics::set_mouse_captured(bool captured) {
  mouse_captured_ = captured;
}
void UIGraphics::set_key_captured(bool captured) {
  key_captured_ = captured;
}
bool UIGraphics::is_mouse_captured() const {
  return mouse_captured_;
}
bool UIGraphics::is_key_captured() const {
  return key_captured_;
}

moodycamel::ReaderWriterQueue<Action>& UIGraphics::get_action_events() {
  return action_events_;
}