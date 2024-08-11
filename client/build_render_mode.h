#ifndef BUILD_RENDER_MODE_H
#define BUILD_RENDER_MODE_H

#include "build_camera.h"
#include "render_mode.h"

class BuildRenderMode : public RenderMode {
public:
  BuildCamera& get_camera();
  void render(Renderer& renderer) const override;
  void seed_camera(const Camera& camera);
private:
  BuildCamera camera_;
};

#endif