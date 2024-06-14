#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include <unordered_set>
#include <vector>
#include "common.h"
#include "section.h"
#include "types.h"
#include "voxel.h"

class Chunk {
public:
  enum Flags : uint32_t {
    DELETED = 1 << 0,
    NONEMPTY = 1 << 1,
  };

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

  void set_flag(Flags flag);
  void unset_flag(Flags flag);
  bool check_flag(Flags flag) const;

  static Location pos_to_loc(const std::array<double, 3>& position);
  static std::array<int, 3> flat_index_to_3d(int i);

  void compute_lighting(Section& section);
  unsigned char get_lighting(int x, int y, int z) const;
  void set_lighting(int x, int y, int z, unsigned char value);
  void set_lighting(int i, unsigned char value);
  static Int3D to_local(Int3D coord);

  static constexpr int sz_x = Common::chunk_sz_x;
  static constexpr int sz_y = Common::chunk_sz_y;
  static constexpr int sz_z = Common::chunk_sz_z;
  static constexpr int sz = Common::chunk_sz;

  static constexpr unsigned char max_lighting = 0xF;

private:
  static std::array<int, 3> flat_index_to_3d_zxy(int i);

  std::vector<Voxel> voxels_;
  std::vector<unsigned char> lighting_;
  Location location_;
  uint32_t flags_ = 0;
};

#endif