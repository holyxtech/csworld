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

  std::array<double, 3> starting_pos = Common::lat_lng_to_world_pos("-25-20-13", "131-02-17");
  starting_pos[1] = 530;

  camera_.set_position(glm::vec3{starting_pos[0], starting_pos[1], starting_pos[2]});
  player_.set_position(starting_pos[0], starting_pos[1], starting_pos[2]);

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
  player_.set_last_location(loc);
}

void Sim::step() {
  bool new_sections = false;

  Message message;
  auto& q = tcp_client_.get_queue();
  bool success = q.try_dequeue(message);
  while (success) {
    // std::cout << "suc " << message.size() << std::endl;
    auto* update = fbs_update::GetUpdate(message.data());
    switch (update->kind_type()) {
    case fbs_update::UpdateKind_Region:
      new_sections = true;
      auto* region = update->kind_as_Region();
      auto* sections = region->sections();
      for (int i = 0; i < sections->size(); ++i) {
        auto* section_update = sections->Get(i);
        auto* loc = section_update->location();
        /*int elevation = section->elevation();
        int landcover = section->landcover(); */
        /*         std::cout
                  << "Received section at "
                  << loc->x() << "," << loc->y() << " with elevation " << elevation << std::endl; */

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

  std::unique_lock<std::mutex> lock(mutex_);
  auto& pos = player_.get_position();
  lock.unlock();

  auto loc = Chunk::pos_to_loc(pos);
  std::cout << pos[0] << "," << pos[1] << "," << pos[2] << std::endl;
  auto& last_location = player_.get_last_location();
  if (loc != last_location || new_sections) {
    auto& sections = region_.get_sections();
    for (int x = -min_render_distance; x <= min_render_distance; ++x) {
      for (int y = -1; y < 1; ++y) {
        for (int z = -min_render_distance; z <= min_render_distance; ++z) {
          auto location = Location{loc[0] + x, loc[1] + y, loc[2] + z};
          if (!region_.has_chunk(location) && world_generator_.ready_to_fill(location, sections)) {
            Chunk chunk(location[0], location[1], location[2]);
            //            auto start = std::chrono::high_resolution_clock::now();
            world_generator_.fill_chunk(chunk, sections);
            /*             auto end = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                         std::cout<<"fill time: "<<duration.count()<<std::endl; */
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
  player_.set_last_location(loc);
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
  double xpos, ypos;
  glfwGetCursorPos(window_, &xpos, &ypos);

  int mouse_right_state = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT);
  if (mouse_right_state == GLFW_PRESS) {
    auto& cursor_start_pos = Input::instance()->get_cursor_start_pos();
    auto xoff = xpos - cursor_start_pos[0];
    auto yoff = cursor_start_pos[1] - ypos;
    camera_.pan(xoff, yoff);
  }
  Input::instance()->set_cursor_start_pos(xpos, ypos);

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
    if (mouse_right_state == GLFW_PRESS) {
      camera_.move_left();
    } else {
      camera_.turn_left();
    }
  } else if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
    if (mouse_right_state == GLFW_PRESS) {
      camera_.move_right();
    } else {
      camera_.turn_right();
    }
  }
  auto& camera_pos = camera_.get_position();

  {
    std::unique_lock<std::mutex> lock(mutex_);
    bool suc = mesh_mutex_.try_lock();
    if (suc) {
      renderer_.consume_mesh_generator(mesh_generator_);
      player_.set_position(camera_pos[0], camera_pos[1], camera_pos[2]);
      ready_to_mesh_ = true;
      mesh_mutex_.unlock();
    }
  }
  cv_.notify_one();

  renderer_.consume_camera(camera_);
  renderer_.render();
}
