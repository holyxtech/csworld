#ifndef SIM_H
#define SIM_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <unordered_set>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "db_manager.h"
#include "lod_loader.h"
#include "lod_mesh_generator.h"
#include "mesh_generator.h"
#include "player.h"
#include "readerwriterqueue.h"
#include "region.h"
#include "renderer.h"
#include "tcp_client.h"
#include "ui.h"
#include "world.h"
#include "world_generator.h"

class Sim {
public:
  Sim(GLFWwindow* window, TCPClient& tcp_client);
  void step();
  void draw(std::int64_t ms);
  void exit();

private:
  struct WindowEvent {
    enum Kind {
      enable_cursor,
      disable_cursor,
    };
    Kind kind;
  };

  void request_sections(std::vector<Location2D>& locs);

  GLFWwindow* window_;
  TCPClient& tcp_client_;
  Region region_;
  LodLoader lod_loader_;
  World world_;
  WorldGenerator world_generator_;
  MeshGenerator mesh_generator_;
  LodMeshGenerator lod_mesh_generator_;
  Renderer renderer_;
  Camera camera_;
  UI ui_;
  DbManager db_manager_;

  std::mutex mutex_;
  std::mutex mesh_mutex_;
  std::mutex camera_mutex_;
  std::condition_variable cv_;
  bool ready_to_mesh_ = true;
  std::unordered_set<Location2D, Location2DHash> requested_sections_;
  std::unordered_map<Location2D, Section, Location2DHash> sections_;
  std::unordered_set<Location, LocationHash> chunks_to_build_;
  std::unordered_set<Location, LocationHash> sections_to_request_;
  Int3D ray_collision_;
  std::uint32_t step_count_ = 0;
  moodycamel::ReaderWriterQueue<WindowEvent> window_events_;
  bool camera_controlled_ = true;

  static constexpr int render_min_y_offset = -2;
  static constexpr int render_max_y_offset = 1;
  // if region_distance = 2 it breaks (placement/removal)
  static constexpr int region_distance = Region::fill_distance;
  static constexpr int render_distance = region_distance;
  static constexpr int section_distance = render_distance + 2;
  static constexpr int frame_rate_target = 60;
};

#endif