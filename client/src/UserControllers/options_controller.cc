#include "options_controller.h"

OptionsController::OptionsController(Sim& sim, std::unique_ptr<UserController> previous_controller)
    : UserController(sim), previous_controller_(std::move(previous_controller)) {}

void OptionsController::init() {
  auto& window_events = sim_.get_window_events();
  auto& ui = sim_.get_ui();
  window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::enable_cursor});
  ui.set_options_open(true);
}

void OptionsController::end() {
  auto& ui = sim_.get_ui();
  ui.set_options_open(false);
}

void OptionsController::process_input(const InputEvent& event) {
  switch (event.kind) {
  case InputEvent::Kind::KeyButtonEvent: {
    auto& key_button_event = std::any_cast<const KeyButtonEvent&>(event.data);
    if (key_button_event.action != GLFW_PRESS) {
      return;
    }
    if (key_button_event.key == GLFW_KEY_O) {
      next_controller_ = std::move(previous_controller_);
      return;
    }
  } break;
  }
}