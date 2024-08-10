#ifndef VOXEL_H
#define VOXEL_H

#include <cstdint>

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
  bricks,

  voxel_enum_size
};

namespace vops {
  bool is_water(Voxel v);
  bool is_opaque(Voxel v);
  bool is_partially_opaque(Voxel v);
  bool is_cube(Voxel v);
} // namespace vops

enum class CubeTexture {
  dirt,
  grass,
  grass_side,
  water,
  sand,
  tree_trunk,
  leaves,
  sandstone,
  stone,
  bricks,

  num_cube_textures,
};

enum class IrregularTexture {
  standing_grass = static_cast<std::uint32_t>(CubeTexture::num_cube_textures),
  roses,
  sunflower,

  num_voxel_textures
};

namespace VoxelTextures {
  constexpr int num_cube_textures = static_cast<int>(CubeTexture::num_cube_textures);
  constexpr int num_voxel_textures = static_cast<int>(IrregularTexture::num_voxel_textures);
  constexpr int num_irregular_textures = num_voxel_textures - static_cast<int>(CubeTexture::num_cube_textures);
} // namespace VoxelTextures

struct VT {
  std::uint32_t v_;
  std::uint32_t get() { return v_; };
  constexpr VT(std::uint32_t v = 1) : v_{v} {}
  constexpr VT(CubeTexture t) : v_{static_cast<std::uint32_t>(t)} {}
  constexpr VT(IrregularTexture t) : v_{static_cast<std::uint32_t>(t)} {}
};

#endif