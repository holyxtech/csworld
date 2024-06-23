#include "ui.h"
#include <vector>
#include "render_utils.h"
#include "renderer.h"

UI::UI() {
  std::vector<Item> items = {Item::dirt, Item::stone, Item::sandstone, Item::water};
  std::size_t i = 0;
  for (; i < items.size(); ++i) {
    action_bar_[i] = items[i];
  }
  for (; i < action_bar_size; ++i)
    action_bar_[i] = Item::empty;
  std::size_t index = 0;
  actions_.emplace_back(Action{Action::new_active_item, Action::NewActiveItemData{action_bar_[index]}});
  active_index_ = index;
}

void UI::action_bar_select(std::size_t index) {
  active_index_ = index;
  actions_.emplace_back(Action{Action::new_active_item, Action::NewActiveItemData{action_bar_[index]}});
}

void UI::action_bar_assign(std::size_t index, Item item) {
}

Item UI::inv_select() const {
  return Item::empty;
}

void UI::set_inv_open(bool open) {
  inv_open_ = open;
}

bool UI::is_inv_open() const {
  return inv_open_;
}

void UI::clear_actions() {
  actions_.clear();
}

const std::vector<Action>& UI::get_actions() const {
  return actions_;
}

const std::array<Item, UI::action_bar_size> UI::get_action_bar() const {
  return action_bar_;
}

std::size_t UI::get_active_index() const {
  return active_index_;
}