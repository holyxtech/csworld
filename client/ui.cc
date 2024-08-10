#include "ui.h"
#include <vector>
#include "render_utils.h"
#include "renderer.h"

UI::UI() {
  auto& m = ItemUtils::items;
  std::vector<Item> items = {m.at("dirt"), m.at("water")};
  std::size_t i = 0;
  for (; i < items.size(); ++i) {
    action_bar_[i] = items[i];
  }
  for (; i < action_bar_size; ++i)
    action_bar_[i] = {};
  std::size_t index = 0;
  actions_.emplace_back(Action{Action::new_active_item, Action::NewActiveItemData{action_bar_[index]}});
  active_index_ = index;

  for (int i = 0; i < static_cast<int>(m.size()); ++i) {
    inv_.emplace_back(i);
  }
}

void UI::action_bar_select(std::size_t index) {
  active_index_ = index;
  actions_.emplace_back(Action{Action::new_active_item, Action::NewActiveItemData{action_bar_[index]}});
}

void UI::action_bar_assign(std::size_t index, Item item) {
  action_bar_[index] = item;
}

Item UI::inv_select() const {
  return ItemUtils::items.at("dirt");
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