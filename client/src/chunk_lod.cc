#include "chunk_lod.h"
#include <iostream>

template <LodLevel level>
int ChunkLod<level>::vi(int x, int y, int z) {
  return x + (2 * sz_x) * (y + sz_y * z);
};

template <LodLevel level>
ChunkLod<level>::ChunkLod(const std::vector<Voxel>& voxels) : voxels_(sz, Voxel::empty) {

  for (int y = 0; y < sz_y; ++y) {
    for (int z = 0; z < sz_z; ++z) {
      for (int x = 0; x < sz_x; ++x) {
        auto v1 = static_cast<std::uint32_t>(voxels[vi(x * 2, y, z * 2)]);
        auto v2 = static_cast<std::uint32_t>(voxels[vi(x * 2 + 1, y, z * 2)]);
        auto v3 = static_cast<std::uint32_t>(voxels[vi(x * 2, y, z * 2 + 1)]);
        auto v4 = static_cast<std::uint32_t>(voxels[vi(x * 2 + 1, y, z * 2 + 1)]);
        int v;
        v = v1 * (v1 == v2 || v1 == v3 || v1 == v4);
        v += (v == 0) * v2 * (v2 == v3 || v2 == v4);
        v += (v == 0) * v3 * (v3 == v4);
        v += (v == 0) * v1;
        v += (v == 0) * v2;
        v += (v == 0) * v3;
        v += (v == 0) * v4;
        voxels_[x + sz_x * (y + sz_y * z)] = static_cast<Voxel>(v);
      }
    }
  }

  // v merge
}

template <LodLevel level>
Voxel ChunkLod<level>::get_voxel(int x, int y, int z) const {
  return voxels_[x + sz_x * (y + sz_y * z)];
}

template class ChunkLod<LodLevel::lod1>;
template class ChunkLod<LodLevel::lod2>;
template class ChunkLod<LodLevel::lod3>;
template class ChunkLod<LodLevel::lod4>;

template <LodLevel level>
const std::vector<Voxel> ChunkLod<level>::get_voxels() const {
  return voxels_;
}