#ifndef GROUND_SELECTION_H
#define GROUND_SELECTION_H

#include <unordered_set>
#include <vector>
#include "../types.h"
#include "pawn.h"

class GroundSelection : public Pawn {
public:
  GroundSelection();
  void process_input(const InputEvent& event) override;
  void step() override;

private:
  bool doing_selection_ = false;
  std::unordered_set<Int3D, LocationHash> selected_;
  bool dirty_ = false;
};

#endif