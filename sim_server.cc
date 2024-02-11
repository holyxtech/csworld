#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "chunk.h"
#include "input.h"
#include "sim_server.h"


SimServer::SimServer() {
  int sz_x = 1;
  int sz_y = 1;
  int sz_z = 1;
  for (int x = 0; x < sz_x; ++x) {
    for (int y = 0; y < sz_y; ++y) {
      for (int z = 0; z < sz_z; ++z) {
      }
    }
  }
}

const Region& SimServer::get_region() const {
    return region_;
}

void SimServer::queue_action(int action) {

}

void SimServer::start() {
    while (true) {
        step();
    }
}

void SimServer::step() {
  
}
