#include "ui.h"
#include <vector>
#include "UI/cefmsg.h"
#include "render_utils.h"
#include "renderer.h"

UI::UI() {
  // create vector of item names:

  // std::vector<std::string> item_selector_names;
  for (auto [item_name, item_id] : items::items) {
    // get name of item
    // item_selector_names.push_back(item_name);
    inv_.push_back(item_id);
  }
  active_index_ = 0;
  //  cefmsg::ItemSelectorInit(item_selector_names);
}

void UI::action_bar_select(std::size_t index) {
  active_index_ = index;
  cefmsg::ActionBarActiveIndex(active_index_);
}

void UI::action_bar_assign(std::size_t index, std::optional<Item> item) {
  action_bar_[index] = item;
}

void UI::action_bar_init() {
  std::vector<std::string> item_names = {
    "dirt",
    "bricks",
    "water",
    "stone",
    "sandstone",
    "sunflower",
    "roses",
    "standing_grass",
    "leaves",
    "tree_trunk"};

  std::size_t i = 0;
  for (; i < item_names.size(); ++i) {
    auto item = items::items.at(item_names[i]);
    action_bar_[i] = item;
  }
  for (; i < action_bar_size; ++i)
    action_bar_[i] = {};

  cefmsg::ActionBarActiveIndex(active_index_);
  cefmsg::ActionBarInit(item_names);
}

void UI::item_selector_init() {
  std::vector<std::string> item_selector_names;
  for (auto [item_name, item_id] : items::items) {
    item_selector_names.push_back(item_name);
  }
  cefmsg::ItemSelectorInit(item_selector_names);
};

void UI::set_inv_open(bool open) {
  inv_open_ = open;
  cefmsg::ItemSelectorShow(open);
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
