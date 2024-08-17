#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <memory>
#include "../flag_manager.h"
#include "../scene_component.h"

class Sim;

enum class GameObjectFlags {
  Disabled = (1 << 0)
};

class GameObject : public FlagManager<GameObjectFlags> {
public:
  std::unique_ptr<SceneComponent>& get_scene_component();
  const std::unique_ptr<SceneComponent>& get_scene_component() const;
  void set_scene_component(std::unique_ptr<SceneComponent> scene_component);
  virtual void step();
  static void set_sim(Sim& sim);

protected:
  Sim& get_sim();
  std::unique_ptr<SceneComponent> scene_component_;
private:
  static Sim* sim_;
};

#endif