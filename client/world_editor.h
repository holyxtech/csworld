#ifndef WORLD_EDITOR_H
#define WORLD_EDITOR_H

#include <array>
#include "types.h"
#include "region.h"

class WorldEditor {
public:
  WorldEditor(Region& region);
  void raise(glm::dvec3& pos, glm::dvec3& dir);
  void reset();

private:
  Region& region_;
  static constexpr int heightmap_sz_x = 100;
  static constexpr int heightmap_sz_z = 100;
  static constexpr int heightmap_sz = heightmap_sz_x * heightmap_sz_z;
  std::array<float, heightmap_sz> heightmap_;
  Int2D heightmap_origin_;
  std::array<int, heightmap_sz> voxel_heightmap_;
  static int max_dropoff_distance;
  static int brush_size;
};

#endif