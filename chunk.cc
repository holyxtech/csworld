#include "chunk.h"
#include <iostream>
#include <queue>

Chunk::Chunk(int x, int y, int z)
    : location_{x, y, z}, voxels_(sz, Voxel::empty) {
}

Chunk::Chunk(const Location& loc, const unsigned char* data, int data_size) : location_{loc}, voxels_(sz, Voxel::empty) {
  int vidx = 0;
  for (int i = 0; i < data_size; i += 4) {
    // memory alignment...
    std::uint32_t run = *(int*)(&data[i]);
    auto voxel = static_cast<Voxel>((Common::chunk_data_voxel_mask & run) >> 16);
    std::uint32_t run_length = Common::chunk_data_run_length_mask & run;
    for (int n = 0; n < run_length; ++n) {
      auto [x, y, z] = flat_index_to_3d_zxy(vidx++);
      set_voxel(x, y, z, voxel);
    }
  }
  set_flag(Flags::NONEMPTY);
}

const Location& Chunk::get_location() const {
  return location_;
}

Voxel Chunk::get_voxel(int x, int y, int z) const {
  return voxels_[x + sz_x * (y + sz_y * z)];
}

int Chunk::get_index(int x, int y, int z) {
  return x + sz_x * (y + sz_y * z);
}

int Chunk::get_index(const Int3D& coord) {
  return coord[0] + sz_x * (coord[1] + sz_y * coord[2]);
}

std::array<int, 3> Chunk::flat_index_to_3d_zxy(int i) {
  auto arr = flat_index_to_3d(i);
  auto tmp = arr[0];
  arr[0] = arr[1];
  arr[1] = arr[2];
  arr[2] = tmp;
  return arr;
}

std::array<int, 3> Chunk::flat_index_to_3d(int i) {
  std::array<int, 3> arr;
  arr[0] = i % Chunk::sz_x;
  int j = ((i - arr[0]) / Chunk::sz_x);
  arr[1] = j % Chunk::sz_y;
  arr[2] = (j - arr[1]) / Chunk::sz_y;
  return arr;
}

void Chunk::set_voxel(int i, Voxel voxel) {
  Voxel cur = voxels_[i];
  voxels_[i] = voxel;
}

void Chunk::set_voxel(int x, int y, int z, Voxel voxel) {
  std::size_t i = x + sz_x * (y + sz_y * z);
  set_voxel(i, voxel);
}

Voxel Chunk::get_voxel(int i) const {
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

void Chunk::compute_lighting(Section& section) {
  if (lighting_.size() == 0) {
    lighting_.reserve(sz);
    for (int i = 0; i < sz; ++i)
      lighting_.emplace_back(0);
  } else {
    for (int i = 0; i < sz; ++i)
      lighting_[i] = 0;
  }

  std::queue<Int3D> lights;

  int top_y = sz_y - 1 + location_[1] * sz_y;
  for (int z = 0; z < sz_z; ++z) {
    for (int x = 0; x < sz_x; ++x) {
      int obstructing_height = section.get_subsection_obstructing_height(x, z);

      if (top_y <= obstructing_height) {
        continue;
      }

      int idx = get_index(x, sz_y - 1, z);
      set_lighting(idx, max_lighting);
      lights.emplace(Int3D{x, sz_y - 1, z});
    }
  }

  while (!lights.empty()) {
    auto& coord = lights.front();
    int x = coord[0], y = coord[1], z = coord[2];
    auto lighting = get_lighting(x, y, z);
    lights.pop();

    if (x > 0 && get_voxel(x - 1, y, z) < Voxel::OPAQUE_LOWER &&
        lighting > get_lighting(x - 1, y, z) + 1) {
      set_lighting(x - 1, y, z, lighting - 1);
      lights.emplace(Int3D{x - 1, y, z});
    }

    if ((x < sz_x - 1) && get_voxel(x + 1, y, z) < Voxel::OPAQUE_LOWER &&
        lighting > get_lighting(x + 1, y, z) + 1) {
      set_lighting(x + 1, y, z, lighting - 1);
      lights.emplace(Int3D{x + 1, y, z});
    }

    if (y > 0 && lighting > get_lighting(x, y - 1, z) + 1) {
      auto v = get_voxel(x, y - 1, z);
      if (v < Voxel::OPAQUE_LOWER) {
        lights.emplace(Int3D{x, y - 1, z});
        if (v < Voxel::PARTIAL_OPAQUE_LOWER) {
          set_lighting(x, y - 1, z, lighting);
        } else {
          set_lighting(x, y - 1, z, lighting - 1);
        }
      }
    }

    if ((y < sz_y - 1) && get_voxel(x, y + 1, z) < Voxel::OPAQUE_LOWER &&
        lighting > get_lighting(x, y + 1, z) + 1) {
      set_lighting(x, y + 1, z, lighting - 1);
      lights.emplace(Int3D{x, y + 1, z});
    }

    if (z > 0 && get_voxel(x, y, z - 1) < Voxel::OPAQUE_LOWER &&
        lighting > get_lighting(x, y, z - 1) + 1) {
      set_lighting(x, y, z - 1, lighting - 1);
      lights.emplace(Int3D{x, y, z - 1});
    }
    if ((z < sz_z - 1) && get_voxel(x, y, z + 1) < Voxel::OPAQUE_LOWER &&
        lighting > get_lighting(x, y, z + 1) + 1) {
      set_lighting(x, y, z + 1, lighting - 1);
      lights.emplace(Int3D{x, y, z + 1});
    }
  }
}

void Chunk::set_lighting(int i, unsigned char value) {
  lighting_[i] = value;
};

void Chunk::set_lighting(int x, int y, int z, unsigned char value) {
  int idx = get_index(x, y, z);
  set_lighting(idx, value);
}

unsigned char Chunk::get_lighting(int x, int y, int z) const {
  return lighting_[x + sz_x * (y + sz_y * z)];
}

Int3D Chunk::to_local(Int3D coord) {
  int x = ((coord[0] % sz_x) + sz_x) % sz_x;
  int y = ((coord[1] % sz_y) + sz_y) % sz_y;
  int z = ((coord[2] % sz_z) + sz_z) % sz_z;
  return Int3D{x, y, z};
}

const std::vector<Voxel> Chunk::get_voxels() const {
  return voxels_;
}