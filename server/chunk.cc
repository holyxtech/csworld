#include "chunk.h"
#include <iostream>

Chunk::Chunk(int x, int y, int z)
    : location_{x, y, z}, voxels_(sz, Voxel::empty) {
}

const Location& Chunk::get_location() const {
  return location_;
}

Voxel::VoxelType Chunk::get_voxel(int x, int y, int z) const {
  return voxels_[x + sz_x * (y + sz_y * z)];
}

void Chunk::set_voxel(int x, int y, int z, Voxel::VoxelType value) {
  std::size_t i = x + sz_x * (y + sz_y * z);
  voxels_[i] = value;
}

Voxel::VoxelType Chunk::get_voxel(int i) const {
  return voxels_[i];
}
