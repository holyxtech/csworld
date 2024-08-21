#include "draw_generator.h"

DrawGenerator::DrawGenerator(Renderer& renderer) : renderer_(renderer) {}

void DrawGenerator::generate_and_dispatch(const SceneComponent& scene_component) {
  DrawCommand command;
  std::uint32_t component_id = scene_component.get_id();
  auto& material = scene_component.get_material();
  auto& shader_name = material.get_name();
  auto& shader = renderer_.get_shader(shader_name);
  command.shader_id = shader.get_id();
  command.vertex_buffer_id = renderer_.get_vertex_buffer_id(component_id);
  auto& indices = scene_component.get_indices();
  if (indices.size() > 0)
    command.index_buffer_id = renderer_.get_index_buffer_id(component_id);
  else
    command.index_buffer_id = -1;
  command.vertex_count = scene_component.get_vertex_count();
  command.index_count = scene_component.get_indices().size();
  command.instance_count = 0;
  command.primitive_type = scene_component.get_primitive_type();
  GLuint model_id = renderer_.get_uniform_id(shader_name, "model");
  command.uniforms.emplace_back(model_id, UniformType::Matrix4f, (void*)&scene_component.get_model_transform());
  auto& texture_binding_points = shader.get_texture_binding_points();
  for (auto& [name, binding_point] : texture_binding_points) {
    GLuint texture_id = renderer_.get_texture(name);
    command.texture_bindings.emplace_back(texture_id, binding_point);
  }
  auto& uniform_buffer_binding_points = shader.get_uniform_buffer_binding_points();
  for (auto& [name, binding_point] : uniform_buffer_binding_points) {
    GLuint buffer_id = renderer_.get_uniform_buffer(name);
    command.uniform_buffer_bindings.emplace_back(buffer_id, binding_point);
  }
  command.blend = material.get_blend_mode() == Material::BlendMode::Opaque;
  command.depth_test = material.get_material_domain() == Material::MaterialDomain::Surface;
  renderer_.render(command);
}