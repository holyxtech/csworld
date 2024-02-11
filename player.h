#ifndef PLAYER_H
#define PLAYER_H

#include <array>
#include <vector>
#include "camera.h"

class Player {
public:
  const std::array<double, 3>& get_position() const {}
  void set_position(double x, double y, double z) {}

private:
  std::array<double, 3> position_;
};

#endif