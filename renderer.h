#ifndef RENDERER_H
#define RENDERER_H

#define GLM_FORCE_LEFT_HANDED
#include <array>
#include <GL/glew.h>
#include "camera.h"
#include "mesh_generator.h"
#include "region.h"
#include "sky.h"
#include "ui.h"
#include "ui_graphics.h"
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
  void consume_ui(UI& ui);
  void set_highlight(Int3D& highlight);
  void render() const;
  const glm::mat4& get_view_matrix() const;
  const glm::mat4& get_projection_matrix() const;
  static float normalize_x(float x);

  static constexpr GLuint window_width = 2560;
  static constexpr GLuint window_height = 1440;
  

private:
  void creation(
    const Location& loc,
    const std::vector<Vertex>* mesh,
    std::unordered_map<GLuint, GLuint>* vbo_to_vao,
    std::unordered_map<GLuint, Location>* vbo_map,
    std::unordered_map<Location, GLuint, LocationHash>* loc_map,
    std::unordered_map<GLuint, int>* mesh_size_map);

  std::unordered_map<GLuint, GLuint> vbo_to_vao_;
  std::unordered_map<GLuint, Location> vbo_map_;
  std::unordered_map<Location, GLuint, LocationHash> loc_map_;
  std::unordered_map<GLuint, int> mesh_size_map_;
  std::unordered_map<GLuint, GLuint> water_vbo_to_vao_;
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
  GLuint voxel_highlight_vbo_;
  GLuint voxel_highlight_vao_;

  GLuint shader_;
  GLuint window_shader_;
  GLuint voxel_highlight_shader_;
  glm::vec3 voxel_highlight_position_;

  glm::dvec3 camera_offset_;
  glm::vec3 camera_world_position_;
  glm::mat4 projection_;
  glm::mat4 view_;
  World& world_;
  Sky sky_;
  UIGraphics ui_;
};

#endif