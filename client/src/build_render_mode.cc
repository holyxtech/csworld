#include "build_render_mode.h"
#include "sim.h"

BuildRenderMode::BuildRenderMode(Sim& sim) : RenderMode(sim) {
  ground_selection_ = std::make_shared<GroundSelection>();
  auto& world = sim.get_world();
  world.add_pawn(ground_selection_);
  auto& scene_component = ground_selection_->get_scene_component();
  auto& renderer = sim_.get_renderer();
  std::uint32_t component_id = renderer.register_scene_component(*scene_component);
  scene_component->set_id(component_id);
}

void BuildRenderMode::render() const {
  auto& renderer = sim_.get_renderer();
  auto& ui_graphics = renderer.get_ui_graphics();
  auto& draw_generator = sim_.get_draw_generator();

  // need to send ground_selection_ to upload vertex data if changed
  auto& scene_component = ground_selection_->get_scene_component();
  if (scene_component->check_flag(SceneComponentFlags::Dirty)) {
    renderer.upload_buffer_data(
      scene_component->get_id(), BufferType::Vertex, 0,
      scene_component->get_vertices().size(), scene_component->get_vertices().data());
    scene_component->unset_flag(SceneComponentFlags::Dirty);
  }
  // renderer.upload_buffer_data();

  renderer.render_scene();

  // generate draw call...
  // draw generator...
  draw_generator.generate_and_dispatch(*scene_component);

  ui_graphics.render_build_ui();
}

BuildCamera& BuildRenderMode::get_camera() {
  return camera_;
}

void BuildRenderMode::seed_camera(const Camera& camera) {
  camera_.set_position(camera.get_position());
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
  ground_selection_->reset();
}
void BuildRenderMode::end() {
  ground_selection_->set_flag(GameObjectFlags::Disabled);
}
const std::shared_ptr<GroundSelection> BuildRenderMode::get_ground_selection() const {
  return ground_selection_;
}