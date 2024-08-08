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
  Renderer(GLFWwindow* window, const UI& ui, const Camera& camera);
  void consume_mesh_generator(MeshGenerator& mesh_generator);
  void consume_lod_mesh_generator(LodMeshGenerator& lod_mesh_generator);
  void consume_camera(const Camera& camera);
  void set_highlight(const Int3D& highlight);
  void render();
  const glm::mat4& get_view_matrix() const;
  const glm::mat4& get_projection_matrix() const;
  const glm::vec3& get_camera_world_position() const;
  GLuint get_shadow_texture() const;
  const Sky& get_sky() const;
  const UIGraphics& get_ui_graphics() const;
  const Camera& get_camera() const;
  static int window_width;
  static int window_height;
  static double aspect_ratio;
  static double fov;
  static constexpr double near_plane = .1;
  static constexpr double far_plane = 1000.;
  bool move_sun = false;
private:
  void shadow_map();

  GLuint main_framebuffer_;
  GLuint water_framebuffer_;
  GLuint composite_framebuffer_;
  GLuint main_cbo_;
  GLuint main_dbo_;
  GLuint water_cbo_;
  GLuint water_dbo_;
  GLuint water_camera_position_;
  GLuint water_camera_normal_;
  GLuint quad_vao_;
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
  const Camera& camera_;

  // shadows
  static constexpr int num_cascades = 3;
  static int shadow_res;
  GLuint shadow_fbo_;
  GLuint shadow_texture_;
  struct ShadowBlock {
    glm::vec3 light_dir;
    float far_plane;
    glm::vec4 cascade_plane_distances[(num_cascades + 3) / 4];
  };
  GLuint shadow_block_ubo_;
  GLuint light_space_matrices_ubo_;
  static std::array<float, 3> cascade_far_planes;

  static GLuint blur_texture_width;
  static GLuint blur_texture_height;
};

#endif