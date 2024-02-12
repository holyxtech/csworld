#include "chunk.h"
#include <iostream>

Chunk::Chunk(int x, int y, int z)
    : location_{x, y, z}, voxels_(sz, Voxel::empty) {
}

Chunk::Chunk(Location location) : location_{location[0], location[1], location[2]}, voxels_(sz, Voxel::empty) {}

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

Location Chunk::pos_to_loc(const std::array<double, 3>& position) {
  return Location{static_cast<int>(position[0]) / sz_x, static_cast<int>(position[1]) / sz_y, static_cast<int>(position[2]) / sz_z};
}