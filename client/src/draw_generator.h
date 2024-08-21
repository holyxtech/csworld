#ifndef DRAW_GENERATOR_H
#define DRAW_GENERATOR_H

#include "renderer.h"
#include "scene_component.h"

class DrawGenerator {
public:
  DrawGenerator(Renderer& renderer);
  void generate_and_dispatch(const SceneComponent& scene_component);

private:
  Renderer& renderer_;
};

#endif