#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include <unordered_set>
#include <vector>
#include "types.h"

class Chunk {
public:
  Chunk(int x, int y, int z);
  const Location& get_location() const;
  Voxel::VoxelType get_voxel(int x, int y, int z) const;
  Voxel::VoxelType get_voxel(int i) const;
  
  void set_voxel(int x, int y, int z, Voxel::VoxelType value);

  static constexpr int sz_uniform = 64;
  static constexpr int sz_x = sz_uniform;
  static constexpr int sz_y = sz_uniform;
  static constexpr int sz_z = sz_uniform;
  static constexpr int sz = sz_x * sz_y * sz_z;

private:
  std::vector<Voxel::VoxelType> voxels_;

  Location location_;

};

#endif