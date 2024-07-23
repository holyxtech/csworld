#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "lod_mesh_generator.h"
#include "mesh_generator.h"
#include "region.h"
#include "sky.h"
#include "terrain_graphics.h"
#include "ui.h"
#include "ui_graphics.h"
#include "world.h"

class Renderer {
public:
  Renderer(GLFWwindow* window, const UI& ui);
  void consume_mesh_generator(MeshGenerator& mesh_generator);
  void consume_lod_mesh_generator(LodMeshGenerator& lod_mesh_generator);
  void consume_camera(const Camera& camera);
  void set_highlight(Int3D& highlight);
  void render();
  const glm::mat4& get_view_matrix() const;
  const glm::mat4& get_projection_matrix() const;
  const glm::vec3 get_camera_world_position() const;
  const Sky& get_sky() const;
  static float normalize_x(float x);
  const UIGraphics& get_ui_graphics() const;

  static const GLuint window_width = 2560;
  static const GLuint window_height = 1440;
  static constexpr double region_far_plane = 100000.f;

private:
  static const GLuint blur_texture_width_ = window_width / 8;
  static const GLuint blur_texture_height_ = window_height / 8;

  GLuint main_framebuffer_;
  GLuint water_framebuffer_;
  GLuint composite_framebuffer_;
  GLuint main_cbo_;
  GLuint main_dbo_;
  GLuint water_cbo_;
  GLuint water_dbo_;
  GLuint g_water_position_;
  GLuint g_water_normal_;
  GLuint quad_vao_;
  GLuint quad_vbo_;
  GLuint voxel_highlight_vbo_;
  GLuint voxel_highlight_vao_;
  std::array<GLuint, 2> composite_cbos_;
  std::array<GLuint, 2> pingpong_framebuffers_;
  std::array<GLuint, 2> pingpong_textures_;

  GLuint shader_;
  GLuint composite_shader_;
  GLuint voxel_highlight_shader_;
  GLuint blur_shader_;
  GLuint final_shader_;

  glm::vec3 voxel_highlight_position_;
  glm::dvec3 camera_offset_;
  glm::vec3 camera_world_position_;
  glm::mat4 projection_;
  glm::mat4 view_;
  Sky sky_;
  UIGraphics ui_graphics_;
  TerrainGraphics terrain_;
  GLFWwindow* window_;
};

#endif