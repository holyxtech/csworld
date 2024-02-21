#ifndef WORLD_H
#define WORLD_H

#include <glm/glm.hpp>

class World {
public:
  World();
  const glm::vec3& get_sun_dir() const;
  const glm::vec3& get_sun_col() const;
  const glm::vec3& get_ambient_col() const;

private:
  glm::vec3 sun_dir_;
  glm::vec3 sun_col_;
  glm::vec3 ambient_col_;
};

#endif