#include "voxel.h"

namespace vops {
  bool is_water(Voxel v) {
    return v < Voxel::WATER_UPPER && v > Voxel::WATER_LOWER;
  }
  
  bool is_opaque(Voxel v) {
    return v > Voxel::OPAQUE_LOWER;
  }

  bool is_partially_opaque(Voxel v) {
    return v < Voxel::OPAQUE_LOWER && v > Voxel::PARTIAL_OPAQUE_LOWER;
  }

  bool is_cube(Voxel v) {
    return v > Voxel::CUBE_LOWER;
  }
} // namespace vops