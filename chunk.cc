#include "chunk.h"
#include <iostream>

Chunk::Chunk(int x, int y, int z)
    : location_{x, y, z}, voxels_(sz, Voxel::empty) {
  water_voxels_.reserve(water_voxels_reserve);
}

// Chunk::Chunk(Location location) : location_{location[0], location[1], location[2]}, voxels_(sz, Voxel::empty) {}

const Location& Chunk::get_location() const {
  return location_;
}

const std::unordered_set<std::size_t>& Chunk::get_water_voxels() const {
  return water_voxels_;
}

Voxel::VoxelType Chunk::get_voxel(int x, int y, int z) const {
  return voxels_[x + sz_x * (y + sz_y * z)];
}

std::array<int, 3> Chunk::flat_index_to_3d(int i) {
  std::array<int, 3> arr;
  arr[0] = i % Chunk::sz_x;
  int j = ((i - arr[0]) / Chunk::sz_x);
  arr[1] = j % Chunk::sz_y;
  arr[2] = (j - arr[1]) / Chunk::sz_y;
  return arr;
}

std::array<Voxel::VoxelType, 6> Chunk::get_adjacent(int x, int y, int z) const {
  std::array<Voxel::VoxelType, 6> adjacent{Voxel::empty};
  if (x > 0) {
    adjacent[Direction::nx] = voxels_[x - 1 + sz_x * (y + sz_y * z)];
  }
  if (x < Chunk::sz_x - 1) {
    adjacent[Direction::px] = voxels_[x + 1 + sz_x * (y + sz_y * z)];
  }
  if (y > 0) {
    adjacent[Direction::ny] = voxels_[x + sz_x * (y - 1 + sz_y * z)];
  }
  if (y < Chunk::sz_y - 1) {
    adjacent[Direction::py] = voxels_[x + sz_x * (y + 1 + sz_y * z)];
  }
  if (z > 0) {
    adjacent[Direction::nz] = voxels_[x + sz_x * (y + sz_y * z - 1)];
  }
  if (z < Chunk::sz_y - 1) {
    adjacent[Direction::pz] = voxels_[x + sz_x * (y + sz_y * z + 1)];
  }
  return adjacent;
}

void Chunk::set_voxel(int x, int y, int z, Voxel::VoxelType value) {
  std::size_t i = x + sz_x * (y + sz_y * z);
  Voxel::VoxelType cur = voxels_[i];
  if (value < Voxel::WATER_UPPER && value > Voxel::WATER_LOWER) {
    water_voxels_.insert(i);
  } else if (cur < Voxel::WATER_UPPER && cur > Voxel::WATER_LOWER) {
    water_voxels_.erase(i);
  }
  voxels_[i] = value;
}

Voxel::VoxelType Chunk::get_voxel(int i) const {
  return voxels_[i];
}

/* void Chunk::set_voxel(int i, Voxel::VoxelType value) {
  voxels_[i] = value;
} */

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