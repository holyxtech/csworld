#ifndef VOXEL_H
#define VOXEL_H

enum class Voxel {
  empty,
  WATER_LOWER,
  water_full,
  WATER_UPPER,

  grass,
  roses,
  sunflower,

  CUBE_LOWER,
  glass,

  PARTIAL_OPAQUE_LOWER,
  leaves,

  OPAQUE_LOWER,
  dirt,
  sand,
  tree_trunk,
  sandstone,
  stone,

  voxel_enum_size
};

namespace vops {
  bool is_water(Voxel v);
  bool is_opaque(Voxel v);
  bool is_partially_opaque(Voxel v);
  bool is_cube(Voxel v);
} // namespace vops

enum class VoxelTexture {
  dirt,
  grass,
  grass_side,
  water,
  sand,
  tree_trunk,
  leaves,
  sandstone,
  stone,
  standing_grass,
  roses,
  sunflower,

  num_voxel_textures
};

#endif