#include "voxel.h"

namespace {
  auto earthen_voxels = []() -> std::bitset<static_cast<std::size_t>(Voxel::voxel_enum_size)> {
    auto bs = std::bitset<static_cast<std::size_t>(Voxel::voxel_enum_size)>();
    auto set = [&bs](Voxel v) {
      bs.set(static_cast<int>(v));
    };
    set(Voxel::dirt);
    set(Voxel::sand);
    set(Voxel::stone);
    set(Voxel::sandstone);
    return bs;
  }
  ();
} // namespace

namespace vops {
  // Mesh tests
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

  // Gameplay tests
  bool is_earthen(Voxel v) {
    return earthen_voxels.test(static_cast<int>(v));
  }
} // namespace vops