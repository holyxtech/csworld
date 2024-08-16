#ifndef SIM_H
#define SIM_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <unordered_set>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "build_render_mode.h"
#include "camera.h"
#include "db_manager.h"
#include "first_person_render_mode.h"
#include "lod_loader.h"
#include "lod_mesh_generator.h"
#include "mesh_generator.h"
#include "player.h"
#include "readerwriterqueue.h"
#include "region.h"
#include "renderer.h"
#include "tcp_client.h"
#include "ui.h"
#include "user_controller.h"
#include "world.h"
#include "world_editor.h"
#include "world_generator.h"
#include "draw_generator.h"

class Sim {
public:
  struct RenderModes {
    RenderModes(Sim& sim) {
      first_person = std::make_shared<FirstPersonRenderMode>(sim);
      build = std::make_shared<BuildRenderMode>(sim);
      cur = first_person;
    }
    std::shared_ptr<RenderMode> cur;
    std::shared_ptr<FirstPersonRenderMode> first_person;
    std::shared_ptr<BuildRenderMode> build;
  };
  struct WindowEvent {
    enum Kind {
      enable_cursor,
      disable_cursor,
    };
    Kind kind;
  };
  Sim(GLFWwindow* window, TCPClient& tcp_client);
  void step(std::int64_t ms);
  void draw(std::int64_t ms);
  void exit();

  World& get_world();
  Region& get_region();
  UI& get_ui();
  Camera& get_camera();
  MeshGenerator& get_mesh_generator();
  Renderer& get_renderer();
  std::mutex& get_camera_mutex();
  std::mutex& get_mesh_mutex();
  GLFWwindow* get_window();
  Int3D& get_ray_collision();
  moodycamel::ReaderWriterQueue<WindowEvent>& get_window_events();
  std::shared_ptr<FirstPersonRenderMode> get_first_person_render_mode();
  RenderModes& get_render_modes();
  WorldEditor& get_world_editor();
  DrawGenerator& get_draw_generator();

  static constexpr int render_min_y_offset = -2;
  static constexpr int render_max_y_offset = 2;
  static constexpr int region_distance = 4;
  static constexpr int section_distance = region_distance + 3;
  static constexpr int max_sections = 2 * 4 * section_distance * section_distance;
  static constexpr int frame_rate_target = 60;
  static constexpr int max_chunks_to_stream_per_step = 5;

private:
  void request_sections(std::vector<Location2D>& locs);
  void stream_chunks();

  GLFWwindow* window_;
  TCPClient& tcp_client_;
  Region region_;
  LodLoader lod_loader_;
  World world_;
  WorldGenerator world_generator_;
  WorldEditor world_editor_;
  MeshGenerator mesh_generator_;
  LodMeshGenerator lod_mesh_generator_;
  Renderer renderer_;
  DrawGenerator draw_generator_;
  UI ui_;
  DbManager db_manager_;
  std::unique_ptr<UserController> user_controller_;
  RenderModes render_modes_;

  std::mutex controller_mutex_;
  std::mutex mutex_;
  std::mutex mesh_mutex_;
  std::mutex camera_mutex_;
  std::condition_variable cv_;
  bool ready_to_mesh_ = true;

  std::unordered_set<Location2D, Location2DHash> requested_sections_;
  std::unordered_map<Location2D, Section, Location2DHash> sections_;
  Int3D ray_collision_;
  moodycamel::ReaderWriterQueue<WindowEvent> window_events_;
  bool player_controlled_ = true;
  std::uint64_t step_ = 0;
};

#endif