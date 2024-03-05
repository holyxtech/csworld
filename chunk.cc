#include "chunk.h"
#include <iostream>

Chunk::Chunk(int x, int y, int z)
    : location_{x, y, z} {
  voxels_.reserve(sz);
  voxels_.insert(voxels_.end(), sz, Voxel::empty);
  water_voxels_.reserve(water_voxels_reserve);
  flags_ = 0;
}

const Location& Chunk::get_location() const {
  return location_;
}

const std::unordered_set<std::size_t>& Chunk::get_water_voxels() const {
  return water_voxels_;
}

Voxel::VoxelType Chunk::get_voxel(int x, int y, int z) const {
  return voxels_[x + sz_x * (y + sz_y * z)];
}

int Chunk::get_index(int x, int y, int z) {
  return x + sz_x * (y + sz_y * z);
}

std::array<int, 3> Chunk::flat_index_to_3d(int i) {
  std::array<int, 3> arr;
  arr[0] = i % Chunk::sz_x;
  int j = ((i - arr[0]) / Chunk::sz_x);
  arr[1] = j % Chunk::sz_y;
  arr[2] = (j - arr[1]) / Chunk::sz_y;
  return arr;
}

void Chunk::set_voxel(int i, Voxel::VoxelType voxel) {
  Voxel::VoxelType cur = voxels_[i];
  if (voxel < Voxel::WATER_UPPER && voxel > Voxel::WATER_LOWER) {
    water_voxels_.insert(i);
  } else if (cur < Voxel::WATER_UPPER && cur > Voxel::WATER_LOWER) {
    water_voxels_.erase(i);
  }
  voxels_[i] = voxel;
}

void Chunk::set_voxel(int x, int y, int z, Voxel::VoxelType voxel) {
  std::size_t i = x + sz_x * (y + sz_y * z);
  set_voxel(i, voxel);
}

Voxel::VoxelType Chunk::get_voxel(int i) const {
  return voxels_[i];
}

void Chunk::set_flag(Flags flag) {
  flags_ |= flag;
}

void Chunk::unset_flag(Flags flag) {
  flags_ &= ~flag;
}

bool Chunk::check_flag(Flags flag) const {
  return flags_ & flag;
}

Location Chunk::pos_to_loc(const std::array<double, 3>& position) {
  return Location{
    static_cast<int>(std::floor(position[0] / Chunk::sz_x)),
    static_cast<int>(std::floor(position[1] / Chunk::sz_y)),
    static_cast<int>(std::floor(position[2] / Chunk::sz_z)),
  };
}
