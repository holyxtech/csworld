#include "disabled_controller.h"
#include "first_person_controller.h"
#include "input.h"

void DisabledController::move_camera() {}
void DisabledController::process_inputs() {
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

    if (key_button_event.key == GLFW_KEY_ESCAPE) {
      window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::disable_cursor});
      next_controller_ = std::move(previous_controller_);
      return;
    }

    success = key_button_events.try_dequeue(key_button_event);
  }
  next_controller_ = std::make_unique<DisabledController>(sim_, std::move(previous_controller_));
}