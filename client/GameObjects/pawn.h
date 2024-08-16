#ifndef PAWN_H
#define PAWN_H

#include "../input.h"
#include "game_object.h"

class Pawn : public GameObject {
public:
  virtual void process_input(const InputEvent& event) = 0;

private:
};

#endif