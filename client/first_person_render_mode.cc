#include "first_person_render_mode.h"
#include "sim.h"

FirstPersonRenderMode::FirstPersonRenderMode(Sim& sim) : RenderMode(sim) {
  voxel_highlight_.set_scene_component(std::make_unique<SceneComponent>());
  auto& scene_component = voxel_highlight_.get_scene_component();
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
  scene_component->set_vertex_count(vertex_arr.size() / 3);
  std::size_t vertex_byte_count = sizeof(float) * vertex_arr.size();
  std::size_t index_byte_count = sizeof(unsigned int) * index_arr.size();
  std::vector<std::uint8_t> vertices;
  std::vector<unsigned int> indices;
  vertices.resize(vertex_byte_count);
  indices.resize(index_arr.size());
  std::memcpy(vertices.data(), vertex_arr.data(), vertex_byte_count);
  std::memcpy(indices.data(), index_arr.data(), index_byte_count);
  scene_component->set_vertices(std::move(vertices));
  scene_component->set_indices(std::move(indices));

  auto& vertex_attributes = scene_component->get_vertex_attributes();
  VertexAttribute vertex_attribute;
  vertex_attribute.type = VertexAttributeType::Float;
  vertex_attribute.component_count = 3;
  vertex_attributes.push_back(vertex_attribute);
  scene_component->set_primitive_type(PrimitiveType::Lines);

  Material material;
  material.set_name("VoxelHighlight");
  material.set_blend_mode(Material::BlendMode::Opaque);
  material.set_material_domain(Material::MaterialDomain::PostProcess);
  material.add_texture("MainDepth");
  scene_component->set_material(material);

  auto& renderer = sim_.get_renderer();
  std::uint32_t component_id = renderer.register_scene_component(*scene_component);
  scene_component->set_id(component_id);
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
  auto& scene_component = voxel_highlight_.get_scene_component();
  scene_component->set_model_transform(glm::translate(glm::mat4(1.f), voxel_highlight_position));
}

void FirstPersonRenderMode::render() const {
  auto& renderer = sim_.get_renderer();
  auto& ui_graphics = renderer.get_ui_graphics();
  auto& draw_generator = sim_.get_draw_generator();
  auto& scene_component = voxel_highlight_.get_scene_component();
  renderer.render_scene();
  draw_generator.generate_and_dispatch(*scene_component);
  ui_graphics.render_first_person_ui();
}

FirstPersonCamera& FirstPersonRenderMode::get_camera() {
  return camera_;
}