#ifndef WORLD_EDITOR_H
#define WORLD_EDITOR_H

#include <array>
#include "types.h"
#include "region.h"

class WorldEditor {
public:
  WorldEditor(Region& region);
  void raise(const glm::dvec3& pos, const glm::dvec3& dir);
  void reset();

private:
  float gaussian_falloff(float distance_squared) const;

  Region& region_;
  static constexpr int heightmap_sz_x = 50;
  static constexpr int heightmap_sz_z = 50;
  static constexpr int heightmap_sz = heightmap_sz_x * heightmap_sz_z;
  std::array<float, heightmap_sz> heightmap_;
  Int2D heightmap_origin_;
  Int2D brush_tip_local_;
  static int max_dropoff_distance;
  static int max_lift;
  static int brush_radius;
  static float brush_strength;
  static float brush_spread;
};

#endif