#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include "GameObjects/game_object.h"
#include "GameObjects/pawn.h"

class World {
public:
  World();
  void add_game_object(std::shared_ptr<GameObject> obj);
  void add_pawn(std::shared_ptr<Pawn> obj);
  void process_input(const InputEvent& event);
  void step();
private:
  std::vector<std::shared_ptr<Pawn>> pawns_;
};

#endif