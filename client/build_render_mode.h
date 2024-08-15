#ifndef BUILD_RENDER_MODE_H
#define BUILD_RENDER_MODE_H

#include "build_camera.h"
#include "render_mode.h"

class BuildRenderMode : public RenderMode {
public:
  BuildRenderMode(Sim& sim) : RenderMode(sim) {}
  BuildCamera& get_camera();
  void render() const override;
  void seed_camera(const Camera& camera);
  void collect_scene_data() override;

private:
  BuildCamera camera_;
};

#endif