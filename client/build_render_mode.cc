#include "build_render_mode.h"
#include "sim.h"

BuildRenderMode::BuildRenderMode(Sim& sim) : RenderMode(sim) {
  ground_selection_ = std::make_shared<GroundSelection>();
  auto& world = sim.get_world();
  world.add_pawn(ground_selection_);
  ground_selection_->set_scene_component(std::make_unique<SceneComponent>());
  auto& scene_component = ground_selection_->get_scene_component();
  auto& vertices = scene_component->get_vertices();
  vertices.resize(1000);
  scene_component->set_flag(SceneComponentFlags::Dynamic);
  scene_component->set_primitive_type(PrimitiveType::Triangles);
  auto& vertex_attributes = scene_component->get_vertex_attributes();
  VertexAttribute vertex_attribute;
  vertex_attribute.type = VertexAttributeType::Float;
  vertex_attribute.component_count = 3;
  vertex_attributes.push_back(vertex_attribute);
  Material material;
  material.set_name("GroundSelection");
  material.set_blend_mode(Material::BlendMode::Opaque);
  material.set_material_domain(Material::MaterialDomain::PostProcess);
  material.add_texture("MainDepth");
  material.add_texture("MainColor");
  scene_component->set_material(material);

  auto& renderer = sim_.get_renderer();
  std::uint32_t component_id = renderer.register_scene_component(*scene_component);
  scene_component->set_id(component_id);
}

void BuildRenderMode::render() const {
  auto& renderer = sim_.get_renderer();
  auto& ui_graphics = renderer.get_ui_graphics();
  auto& draw_generator = sim_.get_draw_generator();

  // need to send ground_selection_ to upload vertex data if changed
  //renderer.upload_buffer_data();

  renderer.render_scene();

  ui_graphics.render_build_ui();
}

BuildCamera& BuildRenderMode::get_camera() {
  return camera_;
}

void BuildRenderMode::seed_camera(const Camera& camera) {
  camera_.set_position(camera.get_position() + glm::dvec3(0, 25, 0));
}

void BuildRenderMode::step() {
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& region = sim_.get_region();
  std::unique_lock<std::mutex> lock(camera_mutex);
  auto& camera_pos = get_camera().get_position();
  auto& player = region.get_player();
  // this is a strange place to do this...
  player.set_position(camera_pos);

}
void BuildRenderMode::init() {
  ground_selection_->unset_flag(GameObjectFlags::Disabled);
}
void BuildRenderMode::end() {
  ground_selection_->set_flag(GameObjectFlags::Disabled);
}