#include "sim.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "./fbs/common_generated.h"
#include "./fbs/request_generated.h"
#include "./fbs/update_generated.h"
#include "chunk.h"
#include "common.h"
#include "input.h"
#include "item.h"
#include "readerwriterqueue.h"
#include "section.h"

Sim::Sim(GLFWwindow* window, TCPClient& tcp_client)
    : window_(window), tcp_client_(tcp_client), renderer_(window, ui_), region_(sections_) {

  std::array<double, 3> starting_pos{4230249, 316, -1220386};
  {
    std::unique_lock<std::mutex> lock(camera_mutex_);
    camera_.set_position(glm::dvec3{starting_pos[0], starting_pos[1], starting_pos[2]});
  }

  auto& player = region_.get_player();
  player.set_position(starting_pos[0], starting_pos[1], starting_pos[2]);

  auto loc = Chunk::pos_to_loc(starting_pos);
  std::vector<Location2D> locs;
  for (int x = -section_distance; x <= section_distance; ++x) {
    for (int z = -section_distance; z <= section_distance; ++z) {
      auto location = Location2D{loc[0] + x, loc[2] + z};
      locs.emplace_back(location);
    }
  }
  for (int x = -render_distance; x <= render_distance; ++x) {
    for (int y = render_min_y_offset; y <= render_max_y_offset; ++y) {
      for (int z = -render_distance; z <= render_distance; ++z) {
        chunks_to_build_.emplace(Location{loc[0] + x, loc[1] + y, loc[2] + z});
      }
    }
  }

  if (locs.size() > 0)
    request_sections(locs);
  player.set_last_location(loc);
}

/*
-request sections?
  -for now, just do a big request at init

-when received, -store in sim.h
-check how close they are to player
-if past the region render distance,...

-keep a map of what "hasn't been rendered but needs to be", update with each new player loc
-just two set differences: delete the stuff that moved out of the render box(es), add the new stuff
-only add something to the map if it hasn't already been rendered (possible to have rendered things outside the box)
-if new sections, or new loc, go through the map and render if all sections are available
-if chunk in region distance, add to region
-always create lod
*/

void Sim::step() {
  bool new_sections = false;
  Message message;
  auto& q = tcp_client_.get_queue();
  bool success = q.try_dequeue(message);
  while (success) {

    auto* update = fbs_update::GetUpdate(message.data());
    switch (update->kind_type()) {
    case fbs_update::UpdateKind_Region:
      new_sections = true;
      auto* region = update->kind_as_Region();
      auto* sections = region->sections();
      for (int i = 0; i < sections->size(); ++i) {

        auto* section_update = sections->Get(i);
        auto* loc = section_update->location();
        auto x = loc->x(), z = loc->y();
        auto location = Location2D{x, z};
        if (!sections_.contains(location)) {
          auto section = Section(section_update);
          sections_.insert({location, std::move(section)});
          requested_sections_.erase(location);
        }
      }
      break;
    }
    success = q.try_dequeue(message);
  }

  auto& player = region_.get_player();

  {
    std::unique_lock<std::mutex> lock(camera_mutex_);
    auto& camera_pos = camera_.get_position();
    player.set_position(camera_pos[0], camera_pos[1], camera_pos[2]);
    auto ray = Region::raycast(camera_);
    ray_collision_ = Int3D{INT_MAX, INT_MAX, INT_MAX};
    for (auto& coord : ray) {
      auto loc = Region::location_from_global_coords(coord[0], coord[1], coord[2]);
      if (region_.has_chunk(loc) && region_.get_voxel(coord[0], coord[1], coord[2]) != Voxel::empty) {
        ray_collision_ = coord;
        break;
      }
    }
  }

  auto& pos = player.get_position();
  auto loc = Chunk::pos_to_loc(pos);
  auto& last_location = player.get_last_location();
  if (loc != last_location || new_sections) {
    for (int x = -render_distance; x <= render_distance; ++x) {
      for (int y = render_max_y_offset; y >= render_min_y_offset; --y) {
        for (int z = -render_distance; z <= render_distance; ++z) {
          auto location = Location{loc[0] + x, loc[1] + y, loc[2] + z};
          /*           if (region_distance < std::abs(x) || region_distance < (std::abs(z))) {
                      if (lod_loader_.has_lods(location))
                        continue;
                      else if (world_generator_.ready_to_fill(location, sections_)) {
                        Chunk chunk(location[0], location[1], location[2]);
                        world_generator_.fill_chunk(chunk, sections_);
                        lod_loader_.create_lods(chunk);
                      }
                    } else */
          if (!region_.has_chunk(location) && world_generator_.ready_to_fill(location, sections_)) {
            std::optional<Chunk> chunk;
            auto possible_chunk = db_manager_.load_chunk_if_exists(location);
            if (possible_chunk.has_value()) {
              chunk = possible_chunk.value();
            } else {
              chunk.emplace(location[0], location[1], location[2]);
              world_generator_.fill_chunk(*chunk, sections_);
            }

            /* if (!lod_loader_.has_lods(location))
              lod_loader_.create_lods(chunk); */
            region_.add_chunk(std::move(*chunk));
          }
        }
      }
    }
  }

  if (loc != last_location) {
    std::vector<Location2D> locs;
    // compute difference
    // ...

    for (int x = -section_distance; x <= section_distance; ++x) {
      for (int z = -section_distance; z <= section_distance; ++z) {
        auto location = Location2D{loc[0] + x, loc[2] + z};
        if (!(sections_.contains(location) || requested_sections_.contains(location))) {
          locs.emplace_back(location);
          requested_sections_.insert(location);
        }
      }
    }
    if (locs.size() > 0)
      request_sections(locs);
  }
  player.set_last_location(loc);

  {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [this] { return ready_to_mesh_; });
    }
    std::unique_lock<std::mutex> lock(mesh_mutex_);
    // actions
    // ...
    auto& mouse_button_events = Input::instance()->get_mouse_button_events();
    MouseButtonEvent event;
    bool success = mouse_button_events.try_dequeue(event);
    while (success) {
      if (event.button == GLFW_MOUSE_BUTTON_LEFT && event.action == GLFW_PRESS) {
        {
          std::unique_lock<std::mutex> lock(camera_mutex_);
          region_.raycast_remove(camera_);
        }
      } else if (event.button == GLFW_MOUSE_BUTTON_RIGHT && event.action == GLFW_PRESS) {
        {
          std::unique_lock<std::mutex> lock(camera_mutex_);
          auto item = player.get_active_item();
          if (item == Item::dirt) {
            region_.raycast_place(camera_, Voxel::dirt);
          } else if (item == Item::stone) {
            region_.raycast_place(camera_, Voxel::stone);
          } else if (item == Item::sandstone) {
            region_.raycast_place(camera_, Voxel::sandstone);
          } else if (item == Item::water) {
            region_.raycast_place(camera_, Voxel::water_full);
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

      bool inv_open = ui_.is_inv_open();

      if (inv_open && key_button_event.key >= GLFW_KEY_0 && key_button_event.key <= GLFW_KEY_9) {
        // 1. convert cursor pos to [0,1] x [0,1]
        auto cursor_pos = Input::instance()->get_cursor_pos();
        float xpos = cursor_pos[0] / (Renderer::window_width - 1);
        float ypos = cursor_pos[1] / (Renderer::window_height - 1);
        // 1.5 convert the key to an index 0-(9?)
        // 2. do ui_.select_item() on that spot
        // 3. do ui_.action_bar_assign() on the index/item pair
      }

      if (key_button_event.key == GLFW_KEY_1) {
        ui_.action_bar_select(0);
      } else if (key_button_event.key == GLFW_KEY_2) {
        ui_.action_bar_select(1);
      } else if (key_button_event.key == GLFW_KEY_3) {
        ui_.action_bar_select(2);
      } else if (key_button_event.key == GLFW_KEY_4) {
        ui_.action_bar_select(3);
      } else if (key_button_event.key == GLFW_KEY_P) {
        std::cout << "Player at (" << static_cast<long long>(pos[0]) << "," << static_cast<long long>(pos[1]) << "," << static_cast<long long>(pos[2]) << ")" << std::endl;
      } else if (key_button_event.key == GLFW_KEY_I) {
        ui_.set_inv_open(!inv_open);
        if (inv_open)
          window_events_.enqueue(WindowEvent{WindowEvent::disable_cursor});
        else
          window_events_.enqueue(WindowEvent{WindowEvent::enable_cursor});
      }

      success = key_button_events.try_dequeue(key_button_event);
    }

    auto& actions = ui_.get_actions();
    for (auto& action : actions) {
      if (action.kind == Action::new_active_item) {
        auto& data = std::any_cast<const Action::NewActiveItemData&>(action.data);
        player.set_active_item(data.item);
      }
    }
    ui_.clear_actions();
    mesh_generator_.consume_region(region_);
    lod_mesh_generator_.consume_lod_loader(lod_loader_);

    ready_to_mesh_ = false;
  }
  auto& updated_since_reset = region_.get_updated_since_reset();
  for (auto& loc : updated_since_reset) {
    if (!region_.has_chunk(loc))
      continue;
    auto& chunk = region_.get_chunk(loc);
    db_manager_.save_chunk(chunk);
  }
  region_.reset_updated_since_reset();

  ++step_count_;
}

void Sim::request_sections(std::vector<Location2D>& locs) {
  flatbuffers::FlatBufferBuilder builder(Common::max_msg_buffer_size);

  std::vector<fbs_common::Location2D> locations;

  for (auto& loc : locs) {
    fbs_common::Location2D location(loc[0], loc[1]);
    locations.push_back(location);
  }
  auto sections = builder.CreateVectorOfStructs(locations);
  auto request = fbs_request::CreateRequest(builder, sections);
  fbs_request::FinishSizePrefixedRequestBuffer(builder, request);

  const auto* buffer_pointer = builder.GetBufferPointer();
  const auto buffer_size = builder.GetSize();

  Message message(buffer_size);

  std::memcpy(message.data(), buffer_pointer, buffer_size);

  tcp_client_.write(std::move(message));
}

void Sim::draw(std::int64_t ms) {
  WindowEvent event;
  bool success = window_events_.try_dequeue(event);
  while (success) {
    switch (event.kind) {
    case WindowEvent::disable_cursor: {
      glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      camera_controlled_ = true;
      auto& cursor_pos = Input::instance()->get_cursor_pos();
      Input::instance()->set_prev_cursor_pos(cursor_pos[0], cursor_pos[1]);
    } break;
    case WindowEvent::enable_cursor: {
      glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      camera_controlled_ = false;
    } break;
    }
    success = window_events_.try_dequeue(event);
  }

  if (camera_controlled_) {
    std::unique_lock<std::mutex> lock(camera_mutex_);
    auto& cursor_pos = Input::instance()->get_cursor_pos();
    auto& prev_cursor_pos = Input::instance()->get_prev_cursor_pos();

    if (cursor_pos[0] != prev_cursor_pos[0] || cursor_pos[1] != prev_cursor_pos[1]) {
      camera_.pan(cursor_pos[0] - prev_cursor_pos[0], prev_cursor_pos[1] - cursor_pos[1]);
      Input::instance()->set_prev_cursor_pos(cursor_pos[0], cursor_pos[1]);
    }
    renderer_.set_highlight(ray_collision_);

    if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
      camera_.set_base_translation_speed(3);
    } else {
      camera_.set_base_translation_speed(0.1);
    }

    camera_.scale_translation_speed(ms * frame_rate_target / 1000.0);
    camera_.scale_rotation_speed(ms * frame_rate_target / 1000.0);

    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
      camera_.move_forward();
    } else if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
      camera_.move_backward();
    }
    if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS) {
      camera_.move_up();
    } else if (glfwGetKey(window_, GLFW_KEY_X) == GLFW_PRESS) {
      camera_.move_down();
    }

    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
      camera_.move_left();
    } else if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
      camera_.move_right();
    }
    renderer_.consume_camera(camera_);
  }

  {
    std::unique_lock<std::mutex> lock(mutex_);
    bool suc = mesh_mutex_.try_lock();
    if (suc) {
      renderer_.consume_mesh_generator(mesh_generator_);
      renderer_.consume_lod_mesh_generator(lod_mesh_generator_);
      ready_to_mesh_ = true;
      mesh_mutex_.unlock();
    }
  }
  cv_.notify_one();

  renderer_.render();
}

// make sure no starving after arbitrary thread exit
void Sim::exit() {
  ready_to_mesh_ = true;
  cv_.notify_one();
}