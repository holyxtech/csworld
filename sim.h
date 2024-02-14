#ifndef SIM_H
#define SIM_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"

#include "mesh_generator.h"
#include "player.h"
#include "region.h"
#include "renderer.h"
#include "tcp_client.h"
#include "world_generator.h"

class Sim {
public:
  Sim(GLFWwindow* window, TCPClient& tcp_client);
  void step();
  void draw();

private:
  void get_chunks(std::vector<Location>& locs);

  GLFWwindow* window_;
  TCPClient& tcp_client_;
  Region region_;
  WorldGenerator world_generator_;
  MeshGenerator mesh_generator_;
  Renderer renderer_;
  Player player_;
  Camera camera_;

  static constexpr int min_render_distance = 16;
};

#endif