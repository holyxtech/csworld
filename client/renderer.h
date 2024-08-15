#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "lod_mesh_generator.h"
#include "mesh_generator.h"
#include "region.h"
#include "scene_component.h"
#include "sky.h"
#include "terrain_graphics.h"
#include "ui.h"
#include "ui_graphics.h"
#include "world.h"
#include "scene_component.h"
#include "shader.h"

class Sim;

enum class UniformType {
  Matrix4f,
  Vector3f,
  Float,
  Int,
};
struct UniformBinding {
  int id;
  UniformType type;
  void* data;
};
struct UniformBufferBinding {
  int id;
  int binding_point;
};
struct TextureBinding {
  int id;
  int unit;
};
struct DrawCommand {
  int shader_id;

  int vertex_buffer_id;
  int index_buffer_id;

  int vertex_count;
  int index_count;
  int instance_count;

  PrimitiveType primitive_type;

  std::vector<UniformBinding> uniforms;
  std::vector<UniformBufferBinding> uniform_buffer_bindings;
  std::vector<TextureBinding> texture_bindings;

  bool depth_test;
  bool blend;
};

class Renderer {
public:
  Renderer(Sim& sim);
  void consume_mesh_generator(MeshGenerator& mesh_generator);
  void consume_lod_mesh_generator(LodMeshGenerator& lod_mesh_generator);
  void consume_camera(const Camera& camera);
  void render_scene();
  void render(const DrawCommand& command);
  //void render_voxel_highlight();
  const glm::mat4& get_view_matrix() const;
  const glm::mat4& get_projection_matrix() const;
  const glm::vec3& get_camera_offset_position() const;
  GLuint get_shadow_texture() const;
  const Sky& get_sky() const;
  UIGraphics& get_ui_graphics();
  const Camera& get_camera() const;

  std::uint32_t register_scene_component(const SceneComponent& scene_component);
  GLuint get_texture(const std::string& name) const;
  GLuint get_uniform_buffer(const std::string& name) const;
  const Shader get_shader(const std::string& name) const;
  GLuint get_vertex_buffer_id(std::uint32_t component_id);
  GLuint get_index_buffer_id(std::uint32_t component_id);
  GLint get_uniform_id(const std::string& shader_name, const std::string& uniform_name) const;

  static int window_width;
  static int window_height;
  static double aspect_ratio;
  static double fov;
  static double near_plane;
  static double far_plane;

private:
  void shadow_map();
  void ssao();

  GLuint composite_shader_;
  GLuint blur_shader_;
  GLuint final_shader_;

  std::unordered_map<std::string, GLuint> textures_;
  std::unordered_map<std::string, GLuint> uniform_buffers_;
  std::unordered_map<std::string, Shader> shaders_;
  std::unordered_map<std::uint32_t, GLuint> vertex_buffer_ids_;
  std::unordered_map<std::uint32_t, GLuint> index_buffer_ids_;
  std::unordered_map<GLuint, GLuint> vbo_to_vao_;

  GLuint main_fbo_;
  GLuint main_cbo_;
  GLuint main_dbo_;
  GLuint main_camera_position_;
  GLuint main_camera_normal_;
  GLuint water_fbo_;
  GLuint water_cbo_;
  GLuint water_dbo_;
  GLuint water_camera_position_;
  GLuint water_camera_normal_;
  GLuint composite_fbo_;
  GLuint quad_vao_;
  static GLuint blur_texture_width;
  static GLuint blur_texture_height;
  std::array<GLuint, 2> composite_cbos_;
  std::array<GLuint, 2> pingpong_fbos_;
  std::array<GLuint, 2> pingpong_cbos_;
  struct CommonBlock {
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    glm::mat4 normal_matrix;
  } common_block_;
  GLuint common_ubo_;

  glm::dvec3 world_offset_;
  glm::vec3 camera_offset_position_;
  glm::mat4 projection_;
  glm::mat4 view_;
  Sky sky_;
  UIGraphics ui_graphics_;
  TerrainGraphics terrain_;
  Sim& sim_;

  // ssao
  GLuint ssao_shader_;
  GLuint ssao_fbo_;
  GLuint ssao_cbo_;
  GLuint ssao_blur_shader_;
  GLuint ssao_blur_fbo_;
  GLuint ssao_blur_cbo_;
  GLuint ssao_noise_texture_;

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

  std::uint32_t next_component_id_ = 0;
};

#endif