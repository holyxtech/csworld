#include "disabled_controller.h"
#include "first_person_controller.h"
#include "input.h"

DisabledController::DisabledController(Sim& sim, std::unique_ptr<UserController> previous_controller)
    : UserController(sim), previous_controller_(std::move(previous_controller)) {
}
void DisabledController::init() {
  auto& window_events = sim_.get_window_events();
  window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::enable_cursor});
}
void DisabledController::end() {}

void DisabledController::move_camera() {}
void DisabledController::process_input(const InputEvent& event) {
  auto& window_events = sim_.get_window_events();
  switch (event.kind) {
  case InputEvent::Kind::MouseButtonEvent:
    break;
  case InputEvent::Kind::KeyButtonEvent:
    auto& key_button_event = std::any_cast<const KeyButtonEvent&>(event.data);
    if (key_button_event.action != GLFW_PRESS) {
      return;
    }
    if (key_button_event.key == GLFW_KEY_ESCAPE) {
      next_controller_ = std::move(previous_controller_);
      return;
    }
    break;
  }
  return;
}