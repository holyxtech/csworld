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
  voxels_[x + sz_x * (y + sz_y * z)] = value;
}

Voxel::VoxelType Chunk::get_voxel(int i) const {
  return voxels_[i];
}

void Chunk::set_voxel(int i, Voxel::VoxelType value) {
  voxels_[i] = value;
}

void Chunk::set_flag(Flags flag) {
  flags_ |= flag;
}

void Chunk::unset_flag(Flags flag) {
  flags_ &= ~flag;
}

bool Chunk::check_flag(Flags flag) {
  return flags_ & flag;
}