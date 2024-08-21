#ifndef WORLD_EDITOR_H
#define WORLD_EDITOR_H

#include <unordered_set>
#include <vector>
#include <array>
#include "types.h"

class Sim;

class WorldEditor {
public:
  WorldEditor(Sim& sim);
  void raise(const glm::dvec3& pos, const glm::dvec3& dir);
  void reset();
  void generate(const std::unordered_set<Int3D, LocationHash>& surface);

private:
  float gaussian_falloff(float distance_squared) const;

  Sim& sim_;
  static constexpr int heightmap_sz_x = 10;
  static constexpr int heightmap_sz_z = 10;
  static constexpr int heightmap_sz = heightmap_sz_x * heightmap_sz_z;
  std::vector<float> heightmap_;
  Int2D heightmap_origin_;
  Int2D brush_tip_local_;
  static int max_dropoff_distance;
  static int max_lift;
  static int brush_radius;
  static float brush_strength;
  static float brush_spread;
};

#endif