#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include <unordered_set>
#include <vector>
#include "common.h"
#include "flag_manager.h"
#include "section.h"
#include "types.h"
#include "voxel.h"

enum class ChunkFlags : uint32_t {
  Deleted = 1 << 0,
  Empty = 1 << 1,
};

class Chunk : public FlagManager<ChunkFlags> {
public:
  Chunk(int x, int y, int z);
  Chunk(const Location& loc, const unsigned char* data, int data_size);

  const Location& get_location() const;
  Voxel get_voxel(int x, int y, int z) const;
  Voxel get_voxel(int i) const;
  static int get_index(int x, int y, int z);
  static int get_index(const Int3D& coord);
  const std::vector<Voxel> get_voxels() const;

  void set_voxel(int i, Voxel voxel);
  void set_voxel(int x, int y, int z, Voxel voxel);

  static Location pos_to_loc(const glm::dvec3& position);
  static std::array<int, 3> flat_index_to_3d(int i);

  static Int3D to_local(Int3D coord);

  static constexpr int sz_x = Common::chunk_sz_x;
  static constexpr int sz_y = Common::chunk_sz_y;
  static constexpr int sz_z = Common::chunk_sz_z;
  static constexpr int sz = Common::chunk_sz;

private:
  static std::array<int, 3> flat_index_to_3d_zxy(int i);

  std::vector<Voxel> voxels_;
  Location location_;
  std::uint32_t flags_ = 0;
};

#endif