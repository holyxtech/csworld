#include "sim.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
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

namespace {

} // namespace

Sim::Sim(GLFWwindow* window, TCPClient& tcp_client)
    : window_(window), tcp_client_(tcp_client), renderer_(window, ui_) {

  std::array<double, 3> starting_pos{4230249, 316, -1220386};
  // auto starting_pos = Common::lat_lng_to_world_pos("-25-20-13", "131-02-00");
  // starting_pos[1] = 530;

  {
    std::unique_lock<std::mutex> lock(camera_mutex_);
    camera_.set_position(glm::dvec3{starting_pos[0], starting_pos[1], starting_pos[2]});
  }

  auto& player = region_.get_player();
  player.set_position(starting_pos[0], starting_pos[1], starting_pos[2]);
  auto loc = Chunk::pos_to_loc(starting_pos);

  ++loc[0]; // hack so loc != last_location in step() triggers
  player.set_last_location(loc);
}

void Sim::stream_chunks() {
  int num_new_chunks = 0;
  auto stream_column = [this, &num_new_chunks](Location column) {
    for (int y = render_min_y_offset; y <= render_max_y_offset; ++y) {
      auto location = Location{column[0], column[1] + y, column[2]};
      if (!region_.has_chunk(location) && world_generator_.ready_to_fill(location, sections_)) {
        std::optional<Chunk> chunk;
        auto possible_chunk = db_manager_.load_chunk_if_exists(location);

        if (possible_chunk.has_value()) {
          chunk = possible_chunk.value();
        } else {
          chunk.emplace(location[0], location[1], location[2]);
          world_generator_.fill_chunk(*chunk, sections_);
        }
        region_.add_chunk(std::move(*chunk));

        if (++num_new_chunks == max_chunks_to_stream_per_step)
          return;
      }
    }
  };

  auto& player = region_.get_player();
  auto& pos = player.get_position();
  auto loc = Chunk::pos_to_loc(pos);

  std::array<std::pair<int, int>, 3> column_modifiers = {{
    {1, 0},  // right
    {0, 1},  // up
    {-1, 0}, // left
  }};

  for (int r = 0; r < region_distance; ++r) {
    Location column = Location{loc[0] - r, loc[1], loc[2] - r};
    stream_column(column);
    if (num_new_chunks == max_chunks_to_stream_per_step)
      return;
    for (int i = 0; i < 3; ++i) {
      auto mods = column_modifiers[i];
      for (int moves = 0; moves < 2 * r; ++moves) {
        column[0] += mods.first;
        column[2] += mods.second;
        stream_column(column);
        if (num_new_chunks == max_chunks_to_stream_per_step)
          return;
      }
    }
    for (int moves = 0; moves < 2 * r - 1; ++moves) {
      --column[2];
      stream_column(column);
      if (num_new_chunks == max_chunks_to_stream_per_step)
        return;
    }
  }
}

void Sim::step() {
  bool new_sections = false;
  Message message;
  auto& q = tcp_client_.get_queue();
  bool success = q.try_dequeue(message);
  while (success) {
    auto* update = fbs_update::GetUpdate(message.data());
    switch (update->kind_type()) {
    case fbs_update::UpdateKind_Region: {
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
      if (sections_.size() > max_sections) {
        std::vector<Location2D> section_locs;
        section_locs.reserve(sections_.size());
        for (auto& [location, _] : sections_)
          section_locs.emplace_back(location);
        auto& player = region_.get_player();
        auto& pos = player.get_position();
        auto loc = Chunk::pos_to_loc(pos);
        auto player_location2d = Location2D{loc[0], loc[2]};
        std::sort(
          section_locs.begin(), section_locs.end(),
          [&player_location2d](const Location2D l1, const Location2D l2) {
            auto d1 = LocationMath::distance(l1, player_location2d);
            auto d2 = LocationMath::distance(l2, player_location2d);
            return d1 > d2;
          });
        int to_remove = sections_.size() - max_sections;
        for (int i = 0; i < to_remove; ++i) {
          auto& location = section_locs[i];
          sections_.erase(location);
        }
      }
    } break;
    }
    success = q.try_dequeue(message);
  }

  auto& player = region_.get_player();
  {
    std::unique_lock<std::mutex> lock(camera_mutex_);
    auto& camera_pos = camera_.get_position();
    player.set_position(camera_pos[0], camera_pos[1], camera_pos[2]);
    auto ray = Region::raycast(camera_);
    ray_collision_ = Int3D{std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
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

  // sections
  if (loc != last_location) {
    std::vector<Location2D> locs;
    for (int x = -section_distance; x < section_distance; ++x) {
      for (int z = -section_distance; z < section_distance; ++z) {
        auto location = Location2D{loc[0] + x, loc[2] + z};
        if (!(sections_.contains(location) || requested_sections_.contains(location)))
          locs.emplace_back(location);
      }
    }
    if (locs.size() > 0)
      request_sections(locs);
  }
  stream_chunks();

  player.set_last_location(loc);

  auto& mouse_button_events = Input::instance()->get_mouse_button_events();
  MouseButtonEvent event;
  success = mouse_button_events.try_dequeue(event);
  while (success) {
    if (!player_controlled_) {
      success = mouse_button_events.try_dequeue(event);
      continue;
    }

    if (event.button == GLFW_MOUSE_BUTTON_LEFT && event.action == GLFW_PRESS) {
      {
        std::unique_lock<std::mutex> lock(camera_mutex_);
        region_.raycast_remove(camera_);
      }
    } else if (event.button == GLFW_MOUSE_BUTTON_RIGHT && event.action == GLFW_PRESS) {
      {
        std::unique_lock<std::mutex> lock(camera_mutex_);
        auto item = ui_.get_active_item();
        if (item.has_value()) {
          auto voxel = ItemUtils::item_to_voxel.at(item.value());
          region_.raycast_place(camera_, voxel);
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
      bool inv_open = ui_.is_inv_open();
      ui_.set_inv_open(!inv_open);
      if (inv_open)
        window_events_.enqueue(WindowEvent{WindowEvent::disable_cursor});
      else
        window_events_.enqueue(WindowEvent{WindowEvent::enable_cursor});
    }

    if (ui_.is_inv_open() && key_button_event.key >= GLFW_KEY_0 && key_button_event.key <= GLFW_KEY_9) {
      auto& ui_graphics = renderer_.get_ui_graphics();
      auto hovering = ui_graphics.get_hovering();
      if (hovering.has_value()) {
        std::size_t index = key_button_event.key > GLFW_KEY_0 ? key_button_event.key - GLFW_KEY_1 : UI::action_bar_size - 1;
        ui_.action_bar_assign(index, hovering.value());
      }
    }

    if (!player_controlled_) {
      success = key_button_events.try_dequeue(key_button_event);
      continue;
    }

    if (key_button_event.key >= GLFW_KEY_0 && key_button_event.key <= GLFW_KEY_9) {
      std::size_t index = key_button_event.key > GLFW_KEY_0 ? key_button_event.key - GLFW_KEY_1 : UI::action_bar_size - 1;
      ui_.action_bar_select(index);
    } else if (key_button_event.key == GLFW_KEY_P) {
      std::cout << "Player at (" << static_cast<long long>(pos[0]) << "," << static_cast<long long>(pos[1]) << "," << static_cast<long long>(pos[2]) << ")" << std::endl;
    }

    success = key_button_events.try_dequeue(key_button_event);
  }

  ui_.clear_actions();

  {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [this] { return ready_to_mesh_; });
    }
    std::unique_lock<std::mutex> lock(mesh_mutex_);
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

  ++step_;
}

void Sim::request_sections(std::vector<Location2D>& locs) {
  flatbuffers::FlatBufferBuilder builder(Common::max_msg_buffer_size);

  std::vector<fbs_common::Location2D> locations;

  for (auto& loc : locs) {
    fbs_common::Location2D location(loc[0], loc[1]);
    locations.push_back(location);
    requested_sections_.insert(loc);
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
      player_controlled_ = true;
      auto& cursor_pos = Input::instance()->get_cursor_pos();
      Input::instance()->set_prev_cursor_pos(cursor_pos[0], cursor_pos[1]);
    } break;
    case WindowEvent::enable_cursor: {
      glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      player_controlled_ = false;
    } break;
    }
    success = window_events_.try_dequeue(event);
  }

  if (player_controlled_) {
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

void Sim::exit() {
  ready_to_mesh_ = true;
  cv_.notify_one();
}

void Sim::set_player_controlled(bool controlled) {
  player_controlled_ = controlled;
}