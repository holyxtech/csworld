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
void InventoryController::process_inputs() {
  auto& ui = sim_.get_ui();
  auto& renderer = sim_.get_renderer();
  auto& window_events = sim_.get_window_events();

  auto& mouse_button_events = Input::instance()->get_mouse_button_events();
  MouseButtonEvent event;
  bool success = mouse_button_events.try_dequeue(event);
  while (success) {
    success = mouse_button_events.try_dequeue(event);
  }

  auto& key_button_events = Input::instance()->get_key_button_events();
  KeyButtonEvent key_button_event;
  success = key_button_events.try_dequeue(key_button_event);
  while (success) {
    if (key_button_event.action != GLFW_PRESS) {
      success = key_button_events.try_dequeue(key_button_event);
      continue;
    }

    if (key_button_event.key == GLFW_KEY_I ||
        key_button_event.key == GLFW_KEY_ESCAPE) {
      next_controller_ = std::make_unique<FirstPersonController>(sim_);
      return;
    }

    if (key_button_event.key >= GLFW_KEY_0 && key_button_event.key <= GLFW_KEY_9) {
      auto& ui_graphics = renderer.get_ui_graphics();
      auto hovering = ui_graphics.get_hovering();
      if (hovering.has_value()) {
        std::size_t index = key_button_event.key > GLFW_KEY_0 ? key_button_event.key - GLFW_KEY_1 : UI::action_bar_size - 1;
        ui.action_bar_assign(index, hovering.value());
      }
    }
    success = key_button_events.try_dequeue(key_button_event);
  }
}