#include "build_controller.h"
#include <memory>
#include "first_person_controller.h"
#include "input.h"

void BuildController::move_camera() {
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& camera = sim_.get_render_modes().build->get_camera();
  std::unique_lock<std::mutex> lock(camera_mutex);
  auto& cursor_pos = Input::instance()->get_cursor_pos();
  auto& prev_cursor_pos = Input::instance()->get_prev_cursor_pos();
  if (cursor_pos[0] != prev_cursor_pos[0] || cursor_pos[1] != prev_cursor_pos[1]) {
    Input::instance()->set_prev_cursor_pos(cursor_pos[0], cursor_pos[1]);
  }
}
void BuildController::process_inputs() {
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

    if (key_button_event.key == GLFW_KEY_F) {
      auto& modes = sim_.get_render_modes();
      modes.cur = modes.first_person;
      next_controller_ = std::make_unique<FirstPersonController>(sim_);
      return;
    }

    success = key_button_events.try_dequeue(key_button_event);
  }
}