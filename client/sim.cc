#include "sim.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "build_controller.h"
#include "chunk.h"
#include "common.h"
#include "common_generated.h"
#include "first_person_controller.h"
#include "input.h"
#include "item.h"
#include "readerwriterqueue.h"
#include "request_generated.h"
#include "section.h"
#include "update_generated.h"

Sim::Sim(GLFWwindow* window, TCPClient& tcp_client)
    : window_(window),
      tcp_client_(tcp_client),
      renderer_(*this),
      world_editor_(*this),
      render_modes_(*this),
      draw_generator_(renderer_) {

  {
    GameObject::set_sim(*this);
  }

  user_controller_ = std::make_unique<FirstPersonController>(*this);
  //user_controller_ = std::make_unique<BuildController>(*this);
  user_controller_->init();
  //render_modes_.set_mode(render_modes_.build);

  auto& camera = render_modes_.first_person->get_camera();
  db_manager_.load_camera(camera);

  /*
  //glm::dvec3 starting_pos{4230225.256719, 311.122231, -1220227.127904};
  // auto starting_pos = Common::lat_lng_to_world_pos("-25-20-13", "131-02-00");
  // starting_pos[1] = 530;
    camera.set_position(starting_pos);
   camera.set_orientation(-41.5007, -12); */
  render_modes_.build->seed_camera(camera);

  auto& player = region_.get_player();
  player.set_position(camera.get_position());
  auto loc = Chunk::pos_to_loc(camera.get_position());

  ++loc[0]; // hack so loc != last_location in step() triggers
  player.set_last_location(loc);

  // Can do shader set up...
}

void Sim::stream_chunks() {
  int num_new_chunks = 0;
  auto stream_column = [this, &num_new_chunks](Location column) -> void {
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

  const std::array<std::pair<int, int>, 3> column_modifiers = {{
    {1, 0},  // right
    {0, 1},  // up
    {-1, 0}, // left
  }};

  for (int r = 0; r <= region_distance; ++r) {
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

void Sim::step(std::int64_t ms) {
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
          sections_.insert({location, Section(section_update)});
          requested_sections_.erase(location);
        }
      }
      if (sections_.size() > max_sections) {
        std::vector<Location2D> section_locs;
        section_locs.reserve(sections_.size());
        for (auto& [location, _] : sections_)
          section_locs.push_back(location);
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
  auto& pos = player.get_position();
  auto loc = Chunk::pos_to_loc(pos);
  auto& last_location = player.get_last_location();
  if (loc != last_location) {
    std::vector<Location2D> locs;
    for (int x = -section_distance; x < section_distance; ++x) {
      for (int z = -section_distance; z < section_distance; ++z) {
        auto location = Location2D{loc[0] + x, loc[2] + z};
        if (!(sections_.contains(location) || requested_sections_.contains(location)))
          locs.push_back(location);
      }
    }
    if (locs.size() > 0)
      request_sections(locs);
  }
  stream_chunks();
  player.set_last_location(loc);

  auto process_inputs = [this](auto& event_queue, InputEvent::Kind input_event_kind) {
    using EventType = typename std::remove_reference<decltype(event_queue)>::type::value_type;
    EventType event;
    bool success = event_queue.try_dequeue(event);
    while (success) {
      auto input_event = InputEvent{input_event_kind, event};
      user_controller_->process_input(input_event);
      {
        std::unique_lock<std::mutex> lock(controller_mutex_);
        auto next_controller = user_controller_->get_next_controller();
        if (next_controller != nullptr) {
          user_controller_->end();
          user_controller_ = std::move(next_controller);
          user_controller_->init();
        }
      }
      world_.process_input(input_event);
      success = event_queue.try_dequeue(event);
    }
  };

  auto& ui_graphics = renderer_.get_ui_graphics();
  bool mouse_captured = ui_graphics.is_mouse_captured();
  bool key_captured = ui_graphics.is_key_captured();
  if (mouse_captured || key_captured) {
    world_.interrupt_pawns();
  }
  auto& mouse_button_events = Input::instance()->get_mouse_button_events();
  if (mouse_captured) {
    do { success = mouse_button_events.pop(); } while (success);
  } else {
    process_inputs(mouse_button_events, InputEvent::Kind::MouseButtonEvent);
  }
  auto& key_button_events = Input::instance()->get_key_button_events();
  if (key_captured) {
    do { success = key_button_events.pop(); } while (success);
  } else {
    process_inputs(key_button_events, InputEvent::Kind::KeyButtonEvent);
  }
  auto& ui_action_events = ui_graphics.get_action_events();
  Action event;
  success = ui_action_events.try_dequeue(event);
  while (success) {
    switch (event.kind) {
    case Action::terrain_generation: {
      auto& ground_selection = render_modes_.build->get_ground_selection();
      auto& surface = ground_selection->get_selected();
      world_editor_.generate(surface);
    } break;
    }

    success = ui_action_events.try_dequeue(event);
  }

  world_.step();
  render_modes_.cur->step();

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
      auto& cursor_pos = Input::instance()->get_cursor_pos();
      Input::instance()->set_prev_cursor_pos(cursor_pos[0], cursor_pos[1]);
    } break;
    case WindowEvent::enable_cursor: {
      glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } break;
    }
    success = window_events_.try_dequeue(event);
  }
  {
    std::unique_lock<std::mutex> lock(controller_mutex_);
    user_controller_->move_camera();
  }
  {
    std::unique_lock<std::mutex> lock(camera_mutex_);
    renderer_.consume_camera(get_camera());
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
  render_modes_.cur->render();
}

void Sim::exit() {
  ready_to_mesh_ = true;
  cv_.notify_one();
}
void Sim::save() {
  db_manager_.save_camera(render_modes_.cur->get_camera());
}

Region& Sim::get_region() { return region_; }
UI& Sim::get_ui() { return ui_; }
Camera& Sim::get_camera() {
  auto& camera = render_modes_.cur->get_camera();
  return camera;
}
MeshGenerator& Sim::get_mesh_generator() { return mesh_generator_; }
Renderer& Sim::get_renderer() { return renderer_; }
std::mutex& Sim::get_camera_mutex() { return camera_mutex_; }
std::mutex& Sim::get_mesh_mutex() { return mesh_mutex_; }
GLFWwindow* Sim::get_window() { return window_; }
Int3D& Sim::get_ray_collision() { return ray_collision_; }
moodycamel::ReaderWriterQueue<Sim::WindowEvent>& Sim::get_window_events() { return window_events_; }
std::shared_ptr<FirstPersonRenderMode> Sim::get_first_person_render_mode() { return render_modes_.first_person; }
Sim::RenderModes& Sim::get_render_modes() { return render_modes_; }
WorldEditor& Sim::get_world_editor() { return world_editor_; }
DrawGenerator& Sim::get_draw_generator() { return draw_generator_; }
World& Sim::get_world() { return world_; }
WorldGenerator& Sim::get_world_generator() { return world_generator_; }