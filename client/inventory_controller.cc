#include "inventory_controller.h"
#include <memory>
#include "first_person_controller.h"
#include "input.h"

InventoryController::InventoryController(Sim& sim) : UserController(sim) {
}
void InventoryController::init() {
  auto& ui = sim_.get_ui();
  auto& window_events = sim_.get_window_events();
  ui.set_inv_open(true);
  window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::enable_cursor});
}
void InventoryController::end() {
  auto& ui = sim_.get_ui();
  ui.set_inv_open(false);
}
void InventoryController::move_camera() {}
bool InventoryController::process_input(const InputEvent& event) {
  auto& ui = sim_.get_ui();
  auto& renderer = sim_.get_renderer();
  auto& window_events = sim_.get_window_events();

  switch (event.kind) {
  case InputEvent::Kind::MouseButtonEvent:

    break;
  case InputEvent::Kind::KeyButtonEvent:
    auto& key_button_event = std::any_cast<const KeyButtonEvent&>(event.data);
    if (key_button_event.action != GLFW_PRESS) {
      return false;
    }
    if (key_button_event.key == GLFW_KEY_I ||
        key_button_event.key == GLFW_KEY_ESCAPE) {
      next_controller_ = std::make_unique<FirstPersonController>(sim_);
      return true;
    }
    if (key_button_event.key >= GLFW_KEY_0 && key_button_event.key <= GLFW_KEY_9) {
      auto& ui_graphics = renderer.get_ui_graphics();
      auto hovering = ui_graphics.get_hovering();
      if (hovering.has_value()) {
        std::size_t index = key_button_event.key > GLFW_KEY_0 ? key_button_event.key - GLFW_KEY_1 : UI::action_bar_size - 1;
        ui.action_bar_assign(index, hovering.value());
      }
    }
    break;
  }
  return false;
}