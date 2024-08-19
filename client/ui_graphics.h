#ifndef UI_GRAPHICS_H
#define UI_GRAPHICS_H
#include <array>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "item.h"
#include "readerwriterqueue.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <nuklear.h>
#include "action.h"
#include "ui.h"

class Renderer;

class UIGraphics {
public:
  UIGraphics(GLFWwindow* window, const UI& ui);
  void render_first_person_ui();
  void render_build_ui();
  void set_mouse_captured(bool captured);
  void set_key_captured(bool captured);
  bool is_mouse_captured() const;
  bool is_key_captured() const;
  moodycamel::ReaderWriterQueue<Action>& get_action_events();
  std::optional<Item> get_hovering() const;

private:
  using UITexture = std::uint32_t;

  nk_context* ctx_;
  std::unordered_map<std::string, UITexture> ui_textures;
  std::unordered_map<UITexture, struct nk_image> icons_;
  std::unordered_map<Item, UITexture> item_to_texture_;

  // static float action_bar_padding
  static float action_button_spacing;
  static float action_button_padding;
  static float action_button_border;
  static struct nk_color action_border_default_color;
  static struct nk_color action_border_selected_color;
  static float inv_button_spacing;
  static float inv_button_padding;
  static float inv_scrollbar_width;
  static float inv_button_border;
  static struct nk_color inv_border_hover_color;
  static struct nk_color inv_background_color;
  static struct nk_color build_options_background_color;

  // Write on render thread
  std::atomic<bool> mouse_captured_{false};
  std::atomic<bool> key_captured_{false};

  float font_height_;

  const UI& ui_;

  std::optional<Item> hovering_;
  moodycamel::ReaderWriterQueue<Action> action_events_;
};

#endif