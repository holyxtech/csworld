#ifndef CHUNK_LOD_H
#define CHUNK_LOD_H

#include <array>
#include <utility>
#include <vector>
#include "common.h"
#include "voxel.h"

enum class LodLevel {
  lod1,
  lod2,
  lod3,
  lod4,
};

template <LodLevel Level>
class ChunkLod {
public:
  static constexpr int scale =
    (Level == LodLevel::lod1)   ? 2
    : (Level == LodLevel::lod2) ? 4
    : (Level == LodLevel::lod3) ? 8
                                : 16;
  static constexpr int sz_x = Common::chunk_sz_x / scale;
  static constexpr int sz_z = Common::chunk_sz_z / scale;
  static constexpr int sz_y = Common::chunk_sz_y;
  static constexpr int sz = sz_x * sz_y * sz_z;

  ChunkLod() = default;
  ChunkLod(const std::vector<Voxel>& voxels);
  Voxel get_voxel(int x, int y, int z) const;
  const std::vector<Voxel> get_voxels() const;

private:
  std::vector<Voxel> voxels_;

  int vi(int x, int y, int z);
};

#endif