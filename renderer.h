#ifndef RENDERER_H
#define RENDERER_H

#define GLM_FORCE_LEFT_HANDED
#include <array>
#include <GL/glew.h>
#include "camera.h"
#include "mesh_generator.h"
#include "region.h"
#include "world.h"

typedef struct {
  uint count;
  uint instanceCount;
  uint first;
  uint baseInstance;
} DrawArraysIndirectCommand;

class Renderer {
public:
  Renderer(World& world);
  void consume_mesh_generator(MeshGenerator& mesh_generator);
  void consume_camera(const Camera& camera);
  void render() const;

  static constexpr GLuint window_width = 2560;
  static constexpr GLuint window_height = 1440;

private:
  void activate_vao(GLuint vbo, GLuint vao);

  std::array<GLuint, Region::max_sz> vaos_;
  std::array<GLuint, Region::max_sz> vbos_;
  std::unordered_map<GLuint, Location> vbo_map_;
  std::unordered_map<Location, GLuint, LocationHash> loc_map_;
  std::unordered_map<GLuint, int> mesh_size_map_;

  // Would probably be a better solution to put all the water meshes into a single VBO
  // because if they're greedy meshed, then it would probably take very few kbs on average
  // so can just allocate a spacious VBO up front then render with multidrawarray
  std::array<GLuint, Region::max_sz> water_vaos_;
  std::array<GLuint, Region::max_sz> water_vbos_;
  std::unordered_map<GLuint, Location> water_vbo_map_;
  std::unordered_map<Location, GLuint, LocationHash> water_loc_map_;
  std::unordered_map<GLuint, int> water_mesh_size_map_;

  GLuint main_framebuffer_;
  GLuint water_framebuffer_;
  GLuint main_cbo_;
  GLuint main_dbo_;
  GLuint water_cbo_;
  GLuint water_dbo_;
  GLuint window_vao_;
  GLuint window_vbo_;

  GLuint shader_;
  GLuint window_shader_;

  World& world_;
};

#endif