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
  auto& world_editor = sim_.get_world_editor();
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& camera = sim_.get_render_modes().build->get_camera();

  // check held left mouse
  bool left_pressed = Input::instance()->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
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
    //world_editor.raise(world_space_pos, direction);
  }

  auto& mouse_button_events = Input::instance()->get_mouse_button_events();
  MouseButtonEvent mouse_button_event;
  bool success = mouse_button_events.try_dequeue(mouse_button_event);
  while (success) {
    if (mouse_button_event.action == GLFW_RELEASE) {
      if (mouse_button_event.button == GLFW_MOUSE_BUTTON_LEFT) {
        world_editor.reset();
      }
    }

    success = mouse_button_events.try_dequeue(mouse_button_event);
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