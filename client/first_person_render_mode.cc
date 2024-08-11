#include "first_person_render_mode.h"

void FirstPersonRenderMode::render(Renderer& renderer) const {
  auto& ui_graphics = renderer.get_ui_graphics();
  renderer.render_scene();
  renderer.render_voxel_highlight();
  ui_graphics.render();
}

FirstPersonCamera& FirstPersonRenderMode::get_camera() {
  return camera_;
}