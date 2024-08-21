#include "ui.h"
#include <vector>
#include "render_utils.h"
#include "renderer.h"

UI::UI() {
  auto& m = ItemUtils::items;
  std::vector<Item> items =
    {m.at("dirt"),
     m.at("bricks"),
     m.at("water"),
     m.at("stone"),
     m.at("sandstone"),
     m.at("sunflower"),
     m.at("roses"),
     m.at("standing_grass"),
     m.at("leaves"),
     m.at("tree_trunk")};

  std::size_t i = 0;
  for (; i < items.size(); ++i) {
    action_bar_[i] = items[i];
  }
  for (; i < action_bar_size; ++i)
    action_bar_[i] = {};
  std::size_t index = 0;

  active_index_ = index;

  //  for (int g = 0; g < 20; ++g) {
  for (int i = 0; i < static_cast<int>(m.size()); ++i) {
    inv_.push_back(i);
  }
  //}
}

void UI::action_bar_select(std::size_t index) {
  active_index_ = index;
}

void UI::action_bar_assign(std::size_t index, Item item) {
  action_bar_[index] = item;
}

void UI::set_inv_open(bool open) {
  inv_open_ = open;
}

void UI::set_options_open(bool open) {
  options_open_ = open;
}

bool UI::is_options_open() const {
  return options_open_;
}

bool UI::is_inv_open() const {
  return inv_open_;
}

const std::array<std::optional<Item>, UI::action_bar_size> UI::get_action_bar() const {
  return action_bar_;
}

std::size_t UI::get_active_index() const {
  return active_index_;
}

const std::vector<Item>& UI::get_inv() const {
  return inv_;
}

std::optional<Item> UI::get_active_item() const {
  return action_bar_[active_index_];
}
const UI::Brush& UI::get_brush() const { return brush_; }
void UI::set_brush(Brush&& brush) { brush_ = brush; }
