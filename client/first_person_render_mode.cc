#include "first_person_render_mode.h"
#include "sim.h"

FirstPersonRenderMode::FirstPersonRenderMode(Sim& sim) : RenderMode(sim) {
  std::vector<float> vertex_arr{
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f};
  std::vector<unsigned int> index_arr{
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7};
  voxel_highlight_.set_vertex_count(vertex_arr.size() / 3);
  std::size_t vertex_byte_count = sizeof(float) * vertex_arr.size();
  std::size_t index_byte_count = sizeof(unsigned int) * index_arr.size();
  std::vector<std::uint8_t> vertices;
  std::vector<unsigned int> indices;
  vertices.resize(vertex_byte_count);
  indices.resize(index_arr.size());
  std::memcpy(vertices.data(), vertex_arr.data(), vertex_byte_count);
  std::memcpy(indices.data(), index_arr.data(), index_byte_count);
  voxel_highlight_.set_vertices(std::move(vertices));
  voxel_highlight_.set_indices(std::move(indices));
  auto& v = voxel_highlight_.get_indices();

  std::vector<VertexAttribute> vertex_attributes;
  VertexAttribute vertex_attribute;
  vertex_attribute.type = VertexAttributeType::Float;
  vertex_attribute.component_count = 3;
  vertex_attributes.push_back(vertex_attribute);
  voxel_highlight_.set_vertex_attributes(std::move(vertex_attributes));
  voxel_highlight_.set_primitive_type(PrimitiveType::Lines);

  Material material;
  material.set_name("VoxelHighlight");
  material.set_blend_mode(Material::BlendMode::Opaque);
  material.set_material_domain(Material::MaterialDomain::PostProcess);
  material.add_texture("MainDepth");
  voxel_highlight_.set_material(material);

  auto& renderer = sim_.get_renderer();
  std::uint32_t component_id = renderer.register_scene_component(voxel_highlight_);
  voxel_highlight_.set_id(component_id);
}

void FirstPersonRenderMode::collect_scene_data() {
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& region = sim_.get_region();
  std::unique_lock<std::mutex> lock(camera_mutex);
  auto& camera_pos = get_camera().get_position();
  auto& player = region.get_player();
  auto& mesh_generator = sim_.get_mesh_generator();
  player.set_position(camera_pos);
  auto ray = Region::raycast(camera_pos, get_camera().get_front());
  auto ray_collision = Int3D{std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
  for (auto& coord : ray) {
    auto loc = Region::location_from_global_coord(coord);
    if (region.has_chunk(loc) && region.get_voxel(coord) != Voxel::empty) {
      ray_collision = coord;
      break;
    }
  }
  // set the voxel_highlight transform...
  auto& origin = mesh_generator.get_origin();
  glm::dvec3 world_offset = glm::dvec3(origin[0] * Chunk::sz_x, origin[1] * Chunk::sz_y, origin[2] * Chunk::sz_z);
  glm::vec3 voxel_highlight_position = glm::dvec3(ray_collision[0], ray_collision[1], ray_collision[2]) - world_offset;
  voxel_highlight_.set_model_transform(glm::translate(glm::mat4(1.f), voxel_highlight_position));
}

void FirstPersonRenderMode::render() const {
  auto& renderer = sim_.get_renderer();
  auto& ui_graphics = renderer.get_ui_graphics();
  renderer.render_scene();

  // generate draw commands and pass them off to the renderer...
  DrawCommand command;
  std::uint32_t component_id = voxel_highlight_.get_id();
  auto& material = voxel_highlight_.get_material();
  auto& shader_name = material.get_name();
  auto& shader = renderer.get_shader(shader_name);
  command.shader_id = shader.get_id();
  command.vertex_buffer_id = renderer.get_vertex_buffer_id(component_id);
  auto& indices = voxel_highlight_.get_indices();
  if (indices.size() > 0)
    command.index_buffer_id = renderer.get_index_buffer_id(component_id);
  else
    command.index_buffer_id = -1;
  command.vertex_count = voxel_highlight_.get_vertex_count();
  command.index_count = voxel_highlight_.get_indices().size();
  command.instance_count = 0;
  command.primitive_type = voxel_highlight_.get_primitive_type();
  GLuint model_id = renderer.get_uniform_id(shader_name, "model");
  command.uniforms.emplace_back(model_id, UniformType::Matrix4f, (void*)&voxel_highlight_.get_model_transform());
  auto& texture_binding_points = shader.get_texture_binding_points();
  for (auto& [name, binding_point] : texture_binding_points) {
    GLuint texture_id = renderer.get_texture(name);
    command.texture_bindings.emplace_back(texture_id, binding_point);
  }
  auto& uniform_buffer_binding_points = shader.get_uniform_buffer_binding_points();
  for (auto& [name, binding_point] : uniform_buffer_binding_points) {
    GLuint buffer_id = renderer.get_uniform_buffer(name);
    command.uniform_buffer_bindings.emplace_back(buffer_id, binding_point);
  }
  command.blend = material.get_blend_mode() == Material::BlendMode::Opaque;
  command.depth_test = material.get_material_domain() == Material::MaterialDomain::Surface;
  renderer.render(command);


  ui_graphics.render_first_person_ui();
}

FirstPersonCamera& FirstPersonRenderMode::get_camera() {
  return camera_;
}