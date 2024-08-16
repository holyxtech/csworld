#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <memory>
#include "scene_component.h"

class GameObject {
public:
  std::unique_ptr<SceneComponent>& get_scene_component();
  const std::unique_ptr<SceneComponent>& get_scene_component() const;
  void set_scene_component(std::unique_ptr<SceneComponent> scene_component);

private:
  std::unique_ptr<SceneComponent> scene_component_;
};

#endif