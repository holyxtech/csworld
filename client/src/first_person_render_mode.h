#ifndef FIRST_PERSON_RENDER_MODE_H
#define FIRST_PERSON_RENDER_MODE_H

#include <vector>
#include "first_person_camera.h"
#include "render_mode.h"
#include "GameObjects/game_object.h"
#include "scene_component.h"
#include "shader.h"
  
class FirstPersonRenderMode : public RenderMode {
public:
  FirstPersonRenderMode(Sim& sim);
  FirstPersonCamera& get_camera() override;
  void render() const override;
  void step() override;
  void init() override;
  void end() override;

private:
  FirstPersonCamera camera_;
  GameObject voxel_highlight_;
};

#endif