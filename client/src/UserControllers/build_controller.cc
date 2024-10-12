#include "build_controller.h"
#include <memory>
#include "first_person_controller.h"
#include "input.h"

BuildController::BuildController(Sim& sim) : UserController(sim) {
}
void BuildController::end() {
}
void BuildController::init() {
  auto& modes = sim_.get_render_modes();
  auto& window_events = sim_.get_window_events();
  auto build_mode = modes.build;
  build_mode->seed_camera(modes.first_person->get_camera());
  modes.set_mode(build_mode);
  window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::enable_cursor});
}

void BuildController::move_camera() {
  auto window = sim_.get_window();
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& camera = sim_.get_render_modes().build->get_camera();
  std::unique_lock<std::mutex> lock(camera_mutex);

  auto [xpos, ypos] = Input::instance()->get_cursor_pos();
  auto [prev_xpos, prev_ypos] = Input::instance()->get_prev_cursor_pos();

  bool right_held = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
  if (right_held && (xpos != prev_xpos || ypos != prev_ypos)) {
    camera.rotate(xpos - prev_xpos, prev_ypos - ypos);
  }
  Input::instance()->set_prev_cursor_pos(xpos, ypos);

  
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    camera.set_base_translation_speed(3);
  } else {
    camera.set_base_translation_speed(0.5);
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
void BuildController::process_input(const InputEvent& event) {
  auto& ui = sim_.get_ui();
  auto& renderer = sim_.get_renderer();
  auto& window_events = sim_.get_window_events();
  auto& world_editor = sim_.get_world_editor();
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& camera = sim_.get_render_modes().build->get_camera();
  auto& region = sim_.get_region();

  // check held left mouse
  /* bool left_pressed = Input::instance()->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
  if (left_pressed) {
    auto [mouse_x, mouse_y] = Input::instance()->get_cursor_pos();
    float ndc_x = (2.0f * mouse_x) / Renderer::window_width - 1.0f;
    float ndc_y = 1.0f - (2.0f * mouse_y) / Renderer::window_height;
    glm::dvec4 ndc_space_pos(ndc_x, ndc_y, -1.0f, 1.0f);
    glm::dmat4 inverted_projection = glm::inverse(renderer.get_projection_matrix());
    // this is the position on the near plane with these mouse coordinates
    auto view_space_pos = inverted_projection * ndc_space_pos;
    if (view_space_pos.w != 0)
      view_space_pos /= view_space_pos.w;
    auto inverted_view = glm::inverse(camera.get_raw_view());
    glm::dvec3 world_space_pos = inverted_view * view_space_pos;
    glm::dvec3 direction = glm::normalize(world_space_pos - camera.get_position());
    world_editor.raise(world_space_pos, direction);
  } */
  switch (event.kind) {
  case InputEvent::Kind::MouseButtonEvent:
    break;
  case InputEvent::Kind::KeyButtonEvent:
    auto& key_button_event = std::any_cast<const KeyButtonEvent&>(event.data);
    if (key_button_event.action != GLFW_PRESS) {
      return;
    }
    if (key_button_event.key == GLFW_KEY_F) {
      next_controller_ = std::make_unique<FirstPersonController>(sim_);
      return;
    }
    if (key_button_event.key == GLFW_KEY_Z) {
      // undo
      region.undo_last_update();
    }
    break;
  }
  return;
}