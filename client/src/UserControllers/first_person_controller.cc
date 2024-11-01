#include "first_person_controller.h"
#include "build_controller.h"
#include "disabled_controller.h"
#include "input.h"
#include "inventory_controller.h"
#include "options_controller.h"
#include "UI/cefmsg.h"
#include "UI/cefui.h"

FirstPersonController::FirstPersonController(Sim& sim) : UserController(sim) {
}
void FirstPersonController::end() {
}
void FirstPersonController::init() {
  auto& window_events = sim_.get_window_events();
  auto& modes = sim_.get_render_modes();
  if (modes.cur == modes.build) {
    auto& build_camera = modes.build->get_camera();
    auto& camera = modes.first_person->get_camera();
    camera.set_orientation(build_camera.get_yaw(), build_camera.get_pitch());
    camera.set_position(build_camera.get_position());
  }
  modes.set_mode(modes.first_person);
  window_events.enqueue(Sim::WindowEvent{Sim::WindowEvent::disable_cursor});
  cefui::SetEnableInput(false);
  cefmsg::ViewChange("firstPerson");
}

void FirstPersonController::move_camera() {
  auto window = sim_.get_window();
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& renderer = sim_.get_renderer();
  auto& ray_collision = sim_.get_ray_collision();
  auto& window_events = sim_.get_window_events();
  auto& camera = sim_.get_first_person_render_mode()->get_camera();

  std::unique_lock<std::mutex> lock(camera_mutex);
  auto [xpos, ypos] = Input::instance()->get_cursor_pos();
  auto [prev_xpos, prev_ypos] = Input::instance()->get_prev_cursor_pos();

  if (xpos != prev_xpos || ypos != prev_ypos) {
    camera.pan(xpos - prev_xpos, prev_ypos - ypos);
    Input::instance()->set_prev_cursor_pos(xpos, ypos);
  }

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    camera.set_base_translation_speed(3);
  } else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    camera.set_base_translation_speed(0.06);
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

void FirstPersonController::process_input(const InputEvent& event) {
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

  switch (event.kind) {
  case InputEvent::Kind::MouseButtonEvent: {
    auto& mouse_button_event = std::any_cast<const MouseButtonEvent&>(event.data);
    if (mouse_button_event.button == GLFW_MOUSE_BUTTON_LEFT && mouse_button_event.action == GLFW_PRESS) {
      {
        std::unique_lock<std::mutex> lock(camera_mutex);
        region.raycast_remove(camera);
      }
    } else if (mouse_button_event.button == GLFW_MOUSE_BUTTON_RIGHT && mouse_button_event.action == GLFW_PRESS) {
      {
        std::unique_lock<std::mutex> lock(camera_mutex);
        auto item = ui.get_active_item();
        if (item.has_value()) {
          auto voxel = items::item_to_voxel.at(item.value());
          region.raycast_place(camera.get_position(), camera.get_front(), voxel);
        }
      }
    }
  } break;
  case InputEvent::Kind::KeyButtonEvent: {
    auto& key_button_event = std::any_cast<const KeyButtonEvent&>(event.data);

    if (key_button_event.action != GLFW_PRESS) {
      return;
    }

    if (key_button_event.key == GLFW_KEY_I) {
      next_controller_ = std::make_unique<InventoryController>(sim_);
      return;
    }

    if (key_button_event.key == GLFW_KEY_F) {
      next_controller_ = std::make_unique<BuildController>(sim_);
      return;
    }

    if (key_button_event.key == GLFW_KEY_ESCAPE) {
      next_controller_ = std::make_unique<DisabledController>(sim_, std::make_unique<FirstPersonController>(sim_));
      return;
    }

    if (key_button_event.key == GLFW_KEY_O) {
      next_controller_ = std::make_unique<OptionsController>(sim_, std::make_unique<FirstPersonController>(sim_));
      return;
    }

    if (key_button_event.key == GLFW_KEY_M) {
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

  } break;
  }
  return;
}