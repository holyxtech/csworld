#include "build_controller.h"
#include <memory>
#include "first_person_controller.h"
#include "input.h"

void BuildController::move_camera() {
  auto window = sim_.get_window();
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& camera = sim_.get_render_modes().build->get_camera();
  std::unique_lock<std::mutex> lock(camera_mutex);
  auto& cursor_pos = Input::instance()->get_cursor_pos();
  auto& prev_cursor_pos = Input::instance()->get_prev_cursor_pos();
  if (cursor_pos[0] != prev_cursor_pos[0] || cursor_pos[1] != prev_cursor_pos[1]) {
    Input::instance()->set_prev_cursor_pos(cursor_pos[0], cursor_pos[1]);
  }

  camera.scale_translation_speed(16.6 * Sim::frame_rate_target / 1000.0);
  camera.scale_rotation_speed(16.6 * Sim::frame_rate_target / 1000.0);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.move_forward();
  } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.move_backward();
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    camera.move_up();
  } else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    camera.move_down();
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.move_left();
  } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.move_right();
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
      window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::disable_cursor});
      next_controller_ = std::make_unique<FirstPersonController>(sim_);
      return;
    }

    success = key_button_events.try_dequeue(key_button_event);
  }
}