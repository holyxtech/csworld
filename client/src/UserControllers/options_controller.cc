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

void OptionsController::move_camera() {
/*   auto window = sim_.get_window();
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& renderer = sim_.get_renderer();
  auto& ray_collision = sim_.get_ray_collision();
  auto& window_events = sim_.get_window_events();
  auto& camera = sim_.get_first_person_render_mode()->get_camera();

  std::unique_lock<std::mutex> lock(camera_mutex);
  auto [xpos, ypos] = Input::instance()->get_cursor_pos();
  auto [prev_xpos, prev_ypos] = Input::instance()->get_prev_cursor_pos();

  if (Input::instance()->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
    if (xpos != prev_xpos || ypos != prev_ypos) {
      camera.pan(xpos - prev_xpos, prev_ypos - ypos);
      Input::instance()->set_prev_cursor_pos(xpos, ypos);
    }
  } */
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
    if (key_button_event.key == GLFW_KEY_P) {
      auto& r = sim_.get_renderer();
      auto& s = r.get_sky();
      auto d = s.get_sun_dir();
      std::cout<<"Sun dir: "<<glm::to_string(d)<<std::endl;
    }
  } break;
  }
}