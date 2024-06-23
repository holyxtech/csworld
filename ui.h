#ifndef UI_H
#define UI_H

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
#include "types.h"

class Renderer;

class UI {
public:
  static constexpr std::size_t action_bar_size = 10;

  UI();
  void action_bar_select(std::size_t index);
  void set_inv_open(bool open);
  void action_bar_assign(std::size_t index, Item item);
  bool is_inv_open() const;
  Item inv_select() const;
  void clear_actions();
  const std::vector<Action>& get_actions() const;
  const std::array<Item, action_bar_size> get_action_bar() const;
  std::size_t get_active_index() const;

private:
  std::vector<Action> actions_;

  std::array<Item, action_bar_size> action_bar_;
  std::size_t active_index_;
  std::array<Item, static_cast<std::size_t>(Item::enum_size) - 1> inv_;
  bool inv_open_ = false;
};

#endif