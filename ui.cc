#include "ui.h"
#include <vector>
#include "render_utils.h"
#include "renderer.h"

UI::UI() {
  action_bar_[0] = Item::dirt;
  action_bar_[1] = Item::stone;
  action_bar_[2] = Item::sandstone;
  size_t index = 0;
  actions_.emplace_back(Action{Action::new_active_item, Action::NewActiveItemData{action_bar_[index]}});
  diffs_.emplace_back(Diff{Diff::action_bar_selection, Diff::ActionBarData{index}});
}

void UI::action_bar_select(size_t index) {
  active_ = action_bar_[index];
  diffs_.emplace_back(Diff{Diff::action_bar_selection, Diff::ActionBarData{index}});
  actions_.emplace_back(Action{Action::new_active_item, Action::NewActiveItemData{action_bar_[index]}});
}

void UI::clear_diffs() {
  diffs_.clear();
}

const std::vector<UI::Diff>& UI::get_diffs() const {
  return diffs_;
}

void UI::clear_actions() {
  actions_.clear();
}

const std::vector<Action>& UI::get_actions() const {
  return actions_;
}