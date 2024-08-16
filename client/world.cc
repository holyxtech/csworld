#include "world.h"

World::World() {}
void World::add_game_object(std::shared_ptr<GameObject> obj) {}
void World::add_pawn(std::shared_ptr<Pawn> obj) {
  pawns_.push_back(obj);
}
void World::process_input(const InputEvent& event) {
  for (auto& pawn : pawns_) {
    if (pawn->check_flag(GameObjectFlags::Disabled))
      continue;
    pawn->process_input(event);
  }
}
void World::step() {
  for (auto& pawn : pawns_) {
    if (pawn->check_flag(GameObjectFlags::Disabled))
      continue;
    pawn->step();
  }
}