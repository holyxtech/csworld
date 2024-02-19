#ifndef TYPES_H
#define TYPES_H

#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

using Location = std::array<int, 3>;

struct LocationMath {
  static double distance(Location l1, Location l2) {
    int x_squared = (l1[0] - l2[0]) * (l1[0] - l2[0]);
    int y_squared = (l1[1] - l2[1]) * (l1[1] - l2[1]);
    int z_squared = (l1[2] - l2[2]) * (l1[2] - l2[2]);
    return std::sqrt(x_squared + y_squared + z_squared);
  }
  static int difference(Location l1, Location l2) {
    return std::abs(l1[0] - l2[0]) + std::abs(l1[1] - l2[1]) + std::abs(l1[2] - l2[2]);
  }
};

struct LocationHash {
  std::size_t operator()(const Location& l) const {
    std::hash<int> h;
    int ret = 3;
    ret ^= h(l[0]) | l[0];
    ret ^= h(l[1]) | l[1];
    ret ^= h(l[2]) | l[2];
    return ret;
  }
};

using Message = std::vector<uint8_t>;
struct MessageWithId {
  Message message;
  int id;
};

namespace Voxel {
  enum VoxelType {
    empty,

    WATER_LOWER,
    water_full,
    WATER_UPPER,

    OPAQUE_LOWER,
    dirt,
    sand,

    num_voxel_types
  };

} // namespace Voxel
#endif
