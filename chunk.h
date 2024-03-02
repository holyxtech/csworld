#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include <unordered_set>
#include <vector>
#include "common.h"
#include "types.h"

class Chunk {
public:
  enum Flags : uint32_t {
    DELETED = 1 << 0,
    NONEMPTY = 1 << 1,
  };

  Chunk(int x, int y, int z);

  const Location& get_location() const;
  const std::unordered_set<std::size_t>& get_water_voxels() const;
  Voxel::VoxelType get_voxel(int x, int y, int z) const;
  Voxel::VoxelType get_voxel(int i) const;

  void set_voxel(int x, int y, int z, Voxel::VoxelType value);

  void set_flag(Flags flag);
  void unset_flag(Flags flag);
  bool check_flag(Flags flag) const;

  static Location pos_to_loc(const std::array<double, 3>& position);
  static std::array<int, 3> flat_index_to_3d(int i);

  static constexpr int sz_x = Common::chunk_sz_x;
  static constexpr int sz_y = Common::chunk_sz_y;
  static constexpr int sz_z = Common::chunk_sz_z;
  static constexpr int sz = Common::chunk_sz;

private:
  std::vector<Voxel::VoxelType> voxels_;
  static constexpr int water_voxels_reserve = sz / 10;
  std::unordered_set<std::size_t> water_voxels_;

  Location location_;

  uint32_t flags_;
};

#endif