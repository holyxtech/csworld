#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <GL/glew.h>
#include "camera.h"
#include "mesh_generator.h"
#include "region.h"

class Renderer {
public:
  Renderer();
  void consume_mesh_generator(MeshGenerator& mesh_generator);
  void consume_camera(const Camera& camera);
  void render() const;

private:
  std::array<GLuint, Region::max_sz> vaos_;
  std::array<GLuint, Region::max_sz> vbos_;
  std::unordered_map<GLuint, Location> vbo_map_;
  std::unordered_map<Location, GLuint, LocationHash> loc_map_;
  std::unordered_map<GLuint, int> mesh_size_map_;
  GLuint shader_;

  // 36 is strictly necessary...
  static constexpr int chunk_max_vertices_ = Chunk::sz;
};

#endif