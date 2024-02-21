#include "world.h"

World::World() {
  sun_dir_ = glm::vec3(-1.f, 1.f, 1.f);
  sun_col_ = glm::vec3(0.4f);
  ambient_col_ = glm::vec3(0.7f);
}

const glm::vec3& World::get_sun_dir() const {
  return sun_dir_;
}

const glm::vec3& World::get_sun_col() const {
  return sun_col_;
}

const glm::vec3& World::get_ambient_col() const {
  return ambient_col_;
}