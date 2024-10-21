#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include <unordered_set>
#include <vector>
#include "common.h"
#include "types.h"

class Chunk {
public:
  Chunk(int x, int y, int z);
  const Location& get_location() const;
  /*  Voxel::VoxelType get_voxel(int x, int y, int z) const;
   Voxel::VoxelType get_voxel(int i) const;

   void set_voxel(int x, int y, int z, Voxel::VoxelType value); */

  static constexpr int sz_x = common::chunk_sz_x;
  static constexpr int sz_y = common::chunk_sz_y;
  static constexpr int sz_z = common::chunk_sz_z;
  static constexpr int sz = common::chunk_sz;

private:
  //  std::vector<Voxel::VoxelType> voxels_;

  Location location_;
};

#endif