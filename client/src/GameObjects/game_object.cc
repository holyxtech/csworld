#include "game_object.h"
#include <cassert>
#include "sim.h"

Sim* GameObject::sim_ = nullptr;

std::unique_ptr<SceneComponent>& GameObject::get_scene_component() { return scene_component_; }
const std::unique_ptr<SceneComponent>& GameObject::get_scene_component() const { return scene_component_; }
void GameObject::set_scene_component(std::unique_ptr<SceneComponent> scene_component) {
  scene_component_ = std::move(scene_component);
}
Sim& GameObject::get_sim() {
  assert(sim_ != nullptr);
  return *sim_;
}
void GameObject::set_sim(Sim& sim) { sim_ = &sim; }
void GameObject::step() {}