#ifndef SIM_H
#define SIM_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"

#include "mesh_generator.h"
#include "player.h"
#include "region.h"
#include "renderer.h"
#include "tcp_client.h"
#include "world_generator.h"
#include "world.h"

class Sim {
public:
  Sim(GLFWwindow* window, TCPClient& tcp_client);
  void step();
  void draw(int64_t ms);


private:
  void get_sections(std::vector<Location2D>& locs);

  GLFWwindow* window_;
  TCPClient& tcp_client_;
  Region region_;
  World world_;
  WorldGenerator world_generator_;
  MeshGenerator mesh_generator_;
  Renderer renderer_;
  Player player_;
  Camera camera_;


  std::mutex mutex_;
  std::mutex mesh_mutex_;
  std::condition_variable cv_;
  bool ready_to_mesh_ = true;

  static constexpr int min_render_distance = 2;
};

#endif