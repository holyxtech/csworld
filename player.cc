#include "player.h"

const std::array<double, 3>& Player::get_position() const {
  return position_;
}

void Player::set_position(double x, double y, double z) {
  position_ = {x, y, z};
}

const Location& Player::get_last_location() const {
  return last_location_;
}
void Player::set_last_location(Location& location) {
  last_location_ = location;
}