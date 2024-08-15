#ifndef FIRST_PERSON_RENDER_MODE_H
#define FIRST_PERSON_RENDER_MODE_H

#include <vector>
#include "first_person_camera.h"
#include "render_mode.h"
#include "scene_component.h"
#include "shader.h"

class FirstPersonRenderMode : public RenderMode {
public:
  FirstPersonRenderMode(Sim& sim);
  FirstPersonCamera& get_camera() override;
  void render() const override;
  void collect_scene_data() override;

private:
  FirstPersonCamera camera_;
  SceneComponent voxel_highlight_;
};

#endif