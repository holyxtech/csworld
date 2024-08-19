#ifndef UI_H
#define UI_H

#include <atomic>
#include <optional>
#include <any>
#include <array>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "item.h"

class UI {
public:
  static constexpr std::size_t action_bar_size = 10;

  struct Brush {
    int brush_radius;
  };

  UI();
  void action_bar_select(std::size_t index);
  void set_inv_open(bool open);
  void action_bar_assign(std::size_t index, Item item);
  bool is_inv_open() const;
  const std::array<std::optional<Item>, action_bar_size> get_action_bar() const;
  std::size_t get_active_index() const;
  const std::vector<Item>& get_inv() const;
  std::optional<Item> get_active_item() const;
  const Brush& get_brush() const;
  void set_brush(Brush&& brush);

private:
  // Write on game thread
  std::array<std::optional<Item>, action_bar_size> action_bar_;
  std::size_t active_index_;
  std::vector<Item> inv_;

  // Write on game thread
  std::atomic<bool> inv_open_{false};

  Brush brush_;
};

#endif