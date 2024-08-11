#ifndef FIRST_PERSON_RENDER_MODE_H
#define FIRST_PERSON_RENDER_MODE_H

#include "first_person_camera.h"
#include "render_mode.h"

class FirstPersonRenderMode : public RenderMode {
public:
  FirstPersonCamera& get_camera() override;
  void render(Renderer& renderer) const override;
private:
  FirstPersonCamera camera_;
};

#endif