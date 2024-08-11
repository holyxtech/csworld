#include "build_render_mode.h"

void BuildRenderMode::render(Renderer& renderer) const {
  // renderer.render();
  auto& ui_graphics = renderer.get_ui_graphics();
  renderer.render_scene();
  //ui_graphics.render();
}

BuildCamera& BuildRenderMode::get_camera() {
  return camera_;
}

void BuildRenderMode::seed_camera(const Camera& camera) {
  camera_.set_position(camera.get_position() + glm::dvec3(0, 5, 0));
  camera_.set_orientation(camera.get_yaw(), camera.get_pitch());
}