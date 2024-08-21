#ifndef PLAYER_H
#define PLAYER_H

#include <array>
#include <vector>
#include "camera.h"
#include "types.h"
#include "glm/ext.hpp"

class Player {
public:
  const glm::dvec3& get_position() const;
  void set_position(double x, double y, double z);
  void set_position(const glm::dvec3& pos);
  const Location& get_last_location() const;
  void set_last_location(Location& location);
  void set_active_item(Item item);
  Item get_active_item();

private:
  glm::dvec3 position_;
  Location last_location_;
  Item active_item_;
};

#endif