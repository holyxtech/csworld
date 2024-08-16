#ifndef BUILD_RENDER_MODE_H
#define BUILD_RENDER_MODE_H

#include <memory>
#include "build_camera.h"
#include "render_mode.h"
#include "GameObjects/ground_selection.h"

class BuildRenderMode : public RenderMode {
public:
  BuildRenderMode(Sim& sim);
  BuildCamera& get_camera();
  void render() const override;
  void seed_camera(const Camera& camera);
  void collect_scene_data() override;

private:
  BuildCamera camera_;

  std::shared_ptr<GroundSelection> ground_selection_;
};

#endif