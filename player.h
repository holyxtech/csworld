#ifndef PLAYER_H
#define PLAYER_H

#include <array>
#include <vector>
#include "camera.h"
#include "types.h"

class Player {
public:
  const std::array<double, 3>& get_position() const;
  void set_position(double x, double y, double z);
  const Location& get_last_location() const;
  void set_last_location(Location& location);

private:
  std::array<double, 3> position_;
  Location last_location_;
};

#endif