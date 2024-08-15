#include "build_render_mode.h"
#include "sim.h"

void BuildRenderMode::render() const {
  // renderer.render();
  auto& renderer = sim_.get_renderer();
  auto& ui_graphics = renderer.get_ui_graphics();
  renderer.render_scene();
  ui_graphics.render_build_ui();
}

BuildCamera& BuildRenderMode::get_camera() {
  return camera_;
}

void BuildRenderMode::seed_camera(const Camera& camera) {
  camera_.set_position(camera.get_position() + glm::dvec3(0, 25, 0));
  // camera_.set_orientation(33.3, -33.3);
  // camera_.set_orientation(90, -89);
}

void BuildRenderMode::collect_scene_data() {
  auto& camera_mutex = sim_.get_camera_mutex();
  auto& region = sim_.get_region();
  std::unique_lock<std::mutex> lock(camera_mutex);
  auto& camera_pos = get_camera().get_position();
  auto& player = region.get_player();
  player.set_position(camera_pos);
}