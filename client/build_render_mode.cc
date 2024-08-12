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
  camera_.set_position(camera.get_position() + glm::dvec3(0, 25, 0));
  camera_.set_orientation(33.3, -33.3);
  //camera_.set_orientation(0, 0);
}