#include "chunk.h"
#include <iostream>
#include <queue>

Chunk::Chunk(int x, int y, int z)
    : location_{x, y, z} {
  voxels_.fill(Voxel::empty);
  water_voxels_.reserve(water_voxels_reserve);

  flags_ = 0;
}

const Location& Chunk::get_location() const {
  return location_;
}

const std::unordered_set<std::size_t>& Chunk::get_water_voxels() const {
  return water_voxels_;
}

Voxel Chunk::get_voxel(int x, int y, int z) const {
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

void Chunk::set_voxel(int i, Voxel voxel) {
  Voxel cur = voxels_[i];
  if (voxel < Voxel::WATER_UPPER && voxel > Voxel::WATER_LOWER) {
    water_voxels_.insert(i);
  } else if (cur < Voxel::WATER_UPPER && cur > Voxel::WATER_LOWER) {
    water_voxels_.erase(i);
  }
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
  lighting_.fill(0);
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