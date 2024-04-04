#include "sim.h"
#include <chrono>
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
#include "readerwriterqueue.h"
#include "section.h"

Sim::Sim(GLFWwindow* window, TCPClient& tcp_client)
    : window_(window), tcp_client_(tcp_client), renderer_(world_) {

  // std::array<double, 3> starting_pos = Common::lat_lng_to_world_pos("-11-0-0", "37-59-03");
  // starting_pos[1] = 350;
  std::array<double, 3> starting_pos{4229159, 326, -1222663};
  // std::array<double, 3> starting_pos{0,0,0};

  {
    std::unique_lock<std::mutex> lock(camera_mutex_);
    camera_.set_position(glm::dvec3{starting_pos[0], starting_pos[1], starting_pos[2]});
  }

  auto& player = region_.get_player();
  player.set_position(starting_pos[0], starting_pos[1], starting_pos[2]);

  auto loc = Chunk::pos_to_loc(starting_pos);
  std::vector<Location2D> locs;
  for (int x = -min_section_distance; x <= min_section_distance; ++x) {
    for (int z = -min_section_distance; z <= min_section_distance; ++z) {
      auto location = Location2D{loc[0] + x, loc[2] + z};
      locs.emplace_back(location);
    }
  }
  if (locs.size() > 0)
    request_sections(locs);
  player.set_last_location(loc);
}


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
        if (!region_.has_section(location)) {
          auto section = Section(section_update);
          region_.add_section(section);
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
        return;
      }
    }
  }

  auto& pos = player.get_position();
  auto loc = Chunk::pos_to_loc(pos);

  /*  if (region_.has_chunk(loc)) {
     int x = std::floor(pos[0]);
     int y = std::floor(pos[1]);
     int z = std::floor(pos[2]);
     auto v = region_.get_voxel(x, y, z);
     // std::cout << x<<","<<y<<","<<z << ": " << (int)v<<std::endl;
   } */

  auto& last_location = player.get_last_location();
  if (loc != last_location || new_sections) {
    auto& sections = region_.get_sections();
    for (int x = -min_render_distance; x <= min_render_distance; ++x) {
      for (int y = 1; y > -2; --y) {
        for (int z = -min_render_distance; z <= min_render_distance; ++z) {
          auto location = Location{loc[0] + x, loc[1] + y, loc[2] + z};
          if (!region_.has_chunk(location) && world_generator_.ready_to_fill(location, sections)) {
            Chunk chunk(location[0], location[1], location[2]);
            world_generator_.fill_chunk(chunk, sections);
            region_.add_chunk(std::move(chunk));
          }
        }
      }
    }
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [this] { return ready_to_mesh_; });
    }
    {
      std::unique_lock<std::mutex> lock(mesh_mutex_);
      mesh_generator_.consume_region(region_);
      ready_to_mesh_ = false;
    }
  }
  if (loc != last_location) {
    std::vector<Location2D> locs;
    for (int x = -min_section_distance; x <= min_section_distance; ++x) {
      for (int z = -min_section_distance; z <= min_section_distance; ++z) {
        auto location = Location2D{loc[0] + x, loc[2] + z};
        if (!(region_.has_section(location) || requested_sections_.contains(location))) {
          locs.emplace_back(location);
          requested_sections_.insert(location);
        }
      }
    }
    if (locs.size() > 0)
      request_sections(locs);
  }
  player.set_last_location(loc);

  // actions
  // ...
  auto& mouse_button_events = Input::instance()->get_mouse_button_events();
  MouseButtonEvent event;
  success = mouse_button_events.try_dequeue(event);
  while (success) {

    if (event.button == GLFW_MOUSE_BUTTON_LEFT && event.action == GLFW_PRESS) {
      {
        std::unique_lock<std::mutex> lock(camera_mutex_);
        region_.raycast_remove(camera_);
      }
      {
        std::unique_lock<std::mutex> lock(mesh_mutex_);
        mesh_generator_.consume_region(region_);
      }
    }

    success = mouse_button_events.try_dequeue(event);
  }
}

void Sim::request_sections(std::vector<Location2D>& locs) {
  flatbuffers::FlatBufferBuilder builder(1048576);

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

void Sim::draw(int64_t ms) {
  {
    std::unique_lock<std::mutex> lock(camera_mutex_);
    int mouse_right_state = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT);
    auto& cursor_pos = Input::instance()->get_cursor_pos();

    auto last_cursor_pos = camera_.get_last_cursor_pos();
    if (cursor_pos[0] != last_cursor_pos[0] || cursor_pos[1] != last_cursor_pos[1]) {
      camera_.pan(cursor_pos[0], cursor_pos[1]);
      camera_.set_last_cursor_pos(cursor_pos[0], cursor_pos[1]);
    }
    renderer_.consume_ray(ray_collision_);

    if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
      camera_.set_base_translation_speed(3);
    } else {
      camera_.set_base_translation_speed(0.2);
    }

    int frame_rate_target = 60;
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
  camera_mutex_.unlock();

  {
    std::unique_lock<std::mutex> lock(mutex_);
    bool suc = mesh_mutex_.try_lock();
    if (suc) {

      renderer_.consume_mesh_generator(mesh_generator_);
      ready_to_mesh_ = true;
      mesh_mutex_.unlock();
    }
  }
  cv_.notify_one();

  renderer_.render();
}
