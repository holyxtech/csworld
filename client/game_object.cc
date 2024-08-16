#include "game_object.h"

std::unique_ptr<SceneComponent>& GameObject::get_scene_component() { return scene_component_; }
const std::unique_ptr<SceneComponent>& GameObject::get_scene_component() const { return scene_component_; }
void GameObject::set_scene_component(std::unique_ptr<SceneComponent> scene_component) {
  scene_component_ = std::move(scene_component);
}