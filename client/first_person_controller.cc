#include "first_person_controller.h"
#include "disabled_controller.h"
#include "input.h"
#include "inventory_controller.h"

void FirstPersonController::move_camera() {
  auto window = sim_.get_window();
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& camera = sim_.get_camera();
  auto& renderer = sim_.get_renderer();
  auto& ray_collision = sim_.get_ray_collision();
  auto& window_events = sim_.get_window_events();

  std::unique_lock<std::mutex> lock(camera_mutex);
  auto& cursor_pos = Input::instance()->get_cursor_pos();
  auto& prev_cursor_pos = Input::instance()->get_prev_cursor_pos();

  if (cursor_pos[0] != prev_cursor_pos[0] || cursor_pos[1] != prev_cursor_pos[1]) {
    camera.pan(cursor_pos[0] - prev_cursor_pos[0], prev_cursor_pos[1] - cursor_pos[1]);
    Input::instance()->set_prev_cursor_pos(cursor_pos[0], cursor_pos[1]);
  }
  renderer.set_highlight(ray_collision);

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    camera.set_base_translation_speed(3);
  } else {
    camera.set_base_translation_speed(0.2);
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

void FirstPersonController::process_inputs() {
  auto& region = sim_.get_region();
  auto& ui = sim_.get_ui();
  auto& camera = sim_.get_camera();
  auto& mesh_generator = sim_.get_mesh_generator();
  auto& renderer = sim_.get_renderer();
  auto& player = region.get_player();
  auto& pos = player.get_position();
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& mesh_mutex = sim_.get_mesh_mutex();
  auto& window_events = sim_.get_window_events();

  auto& mouse_button_events = Input::instance()->get_mouse_button_events();
  MouseButtonEvent event;
  bool success = mouse_button_events.try_dequeue(event);
  while (success) {
    if (event.button == GLFW_MOUSE_BUTTON_LEFT && event.action == GLFW_PRESS) {
      {
        std::unique_lock<std::mutex> lock(camera_mutex);
        region.raycast_remove(camera);
      }
    } else if (event.button == GLFW_MOUSE_BUTTON_RIGHT && event.action == GLFW_PRESS) {
      {
        std::unique_lock<std::mutex> lock(camera_mutex);
        auto item = ui.get_active_item();
        if (item.has_value()) {
          auto voxel = ItemUtils::item_to_voxel.at(item.value());
          region.raycast_place(camera, voxel);
        }
      }
    }
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

    if (key_button_event.key == GLFW_KEY_I) {
      ui.set_inv_open(true);
      window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::enable_cursor});
      next_controller_ = std::make_unique<InventoryController>(sim_);
      return;
    }

    if (key_button_event.key == GLFW_KEY_ESCAPE) {
      window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::enable_cursor});
      next_controller_ = std::make_unique<DisabledController>(sim_, std::make_unique<FirstPersonController>(sim_));
      return;
    }

    if (key_button_event.key >= GLFW_KEY_0 && key_button_event.key <= GLFW_KEY_9) {
      std::size_t index = key_button_event.key > GLFW_KEY_0 ? key_button_event.key - GLFW_KEY_1 : UI::action_bar_size - 1;
      ui.action_bar_select(index);
    } else if (key_button_event.key == GLFW_KEY_P) {
      std::cout << "Player at (" << static_cast<long long>(pos[0]) << "," << static_cast<long long>(pos[1]) << "," << static_cast<long long>(pos[2]) << ")" << std::endl;
      camera.print();
    } else if (key_button_event.key == GLFW_KEY_C) {
      camera.set_position(glm::dvec3{4230225.256719, 311.122231, -1220227.127904});
      camera.set_orientation(-41.5007, -12);
    }

    success = key_button_events.try_dequeue(key_button_event);
  }
  ui.clear_actions();
  next_controller_ = std::make_unique<FirstPersonController>(sim_);
}