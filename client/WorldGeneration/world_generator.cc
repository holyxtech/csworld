#include "world_generator.h"
#include <cstdlib>
#include <iostream>
#include "../poisson_disk_sampling.h"
#include "../region.h"

namespace {
  const std::array<std::array<int, 2>, 9> section_order =
    {{{-1, -1},
      {0, -1},
      {1, -1},
      {-1, 0},
      {0, 0},
      {1, 0},
      {-1, 1},
      {0, 1},
      {1, 1}}};
}

WorldGenerator::WorldGenerator() {
  open_simplex_noise(7, &ctx_);
}

bool WorldGenerator::ready_to_fill(Location& location, const std::unordered_map<Location2D, Section, Location2DHash>& sections) const {
  std::array<int, 5> arr{-2, -1, 0, 1, 2};
  for (auto x : arr) {
    for (auto z : arr) {
      if (!sections.contains(Location2D{location[0] + x, location[2] + z}))
        return false;
    }
  }
  return true;
}

std::vector<std::pair<Int3D, Voxel>> WorldGenerator::build_tree(int x, int y, int z) const {
  std::vector<std::pair<Int3D, Voxel>> parts;
  int i, j, k;

  // the randomness needs to be seeded by x,y,z, so the results don't depend on when this is called
  int tree_height = Common::random_int(5, 8);
  int height_without_leaves;
  if (tree_height >= 7) {
    height_without_leaves = Common::random_int(3, 4);
  } else {
    height_without_leaves = Common::random_int(2, 3);
  }

  i = x, j = y, k = z;
  constexpr std::array<int, 5> arr_1 = {-1, 0, 1};
  constexpr std::array<int, 2> arr_2 = {-2, 2};
  for (int count = height_without_leaves; count < tree_height; ++count) {
    for (auto x : arr_1) {
      for (auto z : arr_1) {
        parts.emplace_back(Int3D{i + x, j + count, k + z}, Voxel::leaves);
      }
    }
    for (auto z : arr_1) {
      for (auto x : arr_2) {
        parts.emplace_back(Int3D{i + x, j + count, k + z}, Voxel::leaves);
      }
    }
    for (auto x : arr_1) {
      for (auto z : arr_2) {
        parts.emplace_back(Int3D{i + x, j + count, k + z}, Voxel::leaves);
      }
    }
  }

  // cross on top
  constexpr std::array<int, 3> arr_3 = {-1, 1};
  for (auto x : arr_3) {
    parts.emplace_back(Int3D{i + x, j + tree_height, k}, Voxel::leaves);
  }
  for (auto z : arr_3) {
    parts.emplace_back(Int3D{i, j + tree_height, k + z}, Voxel::leaves);
  }
  parts.emplace_back(Int3D{i, j + tree_height, k}, Voxel::leaves);
  parts.emplace_back(Int3D{i, j + tree_height + 1, k}, Voxel::leaves);

  // trunk
  i = x, j = y, k = z;
  for (int count = 0; count < tree_height; ++count) {
    parts.emplace_back(Int3D{i, j + count, k}, Voxel::tree_trunk);
  }
  return parts;
}

void WorldGenerator::load_features(Section& section) {
  auto& loc = section.get_location();

  float kRadius = 6;
  auto kXMin = std::array<float, 2>{{0.f, 0.f}};
  auto kXMax = std::array<float, 2>{{Section::sz_x, Section::sz_z}};

  std::hash<int> hasher;
  std::uint32_t seed = 0;
  seed ^= hasher(loc[0]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  seed ^= hasher(loc[1]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  std::uint32_t max_sample_attempts = 30;
  auto samples = thinks::PoissonDiskSampling(kRadius, kXMin, kXMax, max_sample_attempts, seed);

  for (auto s : samples) {
    auto landcover = section.get_landcover(s[0], s[1]);
    if (landcover != Common::LandCover::trees)
      continue;
    int subsection_elevation = section.get_subsection_elevation(s[0], s[1]);
    auto tree = build_tree(loc[0] * Section::sz_x + static_cast<int>(s[0]), subsection_elevation + 1, loc[1] * Section::sz_z + static_cast<int>(s[1]));
    for (auto& [coord, voxel] : tree) {
      section.insert_into_features(coord[0], coord[1], coord[2], voxel);
    }
  }
}

void WorldGenerator::fill_chunk(Chunk& chunk, std::unordered_map<Location2D, Section, Location2DHash>& sections) {
  auto& location = chunk.get_location();

  std::array<int, 3> arr{-1, 0, 1};
  for (auto x : arr) {
    for (auto z : arr) {
      auto section_loc = Location2D{location[0] + x, location[2] + z};
      auto& section = sections.at(section_loc);
      if (!section.has_subsection_elevations())
        section.compute_subsection_elevations(sections);
      if (!section.is_features_loaded()) {
        load_features(section);
        section.set_features_loaded(true);
      }
    }
  }
  auto& section = sections.at(Location2D{location[0], location[2]});

  int empty_subsections = 0;
  int y_global = location[1] * Chunk::sz_y;
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int x = 0; x < Chunk::sz_x; ++x) {
      int height = section.get_subsection_elevation(x, z);
      if (height < y_global) {
        ++empty_subsections;
        continue;
      }

      auto landcover = section.get_landcover(x, z);
      Voxel voxel;
      if (landcover == Common::LandCover::bare) {
        voxel = Voxel::stone;
      } else if (landcover == Common::LandCover::water) {
        voxel = Voxel::water_full;
      } else {
        voxel = Voxel::dirt;
      }

      int y = y_global;
      for (; y < (y_global + Chunk::sz_y) && y <= height; ++y) {
        chunk.set_voxel(x, y - y_global, z, voxel);
      }
      if (landcover != Common::LandCover::bare) {
        double n = noise(x + location[0] * Chunk::sz_x, z + location[2] * Chunk::sz_z);
        if (n > 0.65 && y < (y_global + Chunk::sz_y))
          chunk.set_voxel(x, y - y_global, z, Voxel::grass);
        n = noise(x + location[0] * Chunk::sz_x * 2, z + location[2] * Chunk::sz_z * 2);
        if (n > 0.75 && y < (y_global + Chunk::sz_y))
          chunk.set_voxel(x, y - y_global, z, Voxel::roses);
        n = noise(x + location[0] * Chunk::sz_x * 3, z + location[2] * Chunk::sz_z * 3);
        if (n > 0.75 && y < (y_global + Chunk::sz_y))
          chunk.set_voxel(x, y - y_global, z, Voxel::sunflower);
      }
    }
  }
  if (empty_subsections == Chunk::sz_x * Chunk::sz_z)
    chunk.set_flag(ChunkFlags::Empty);

  int num_features = 0;
  for (auto [x, z] : section_order) {
    auto& section = sections.at(Location2D{location[0] + x, location[2] + z});
    auto& features = section.get_features(location);
    for (auto [idx, voxel] : features)
      chunk.set_voxel(idx, voxel);
    num_features += features.size();
  }
  if (num_features > 0)
    chunk.unset_flag(ChunkFlags::Empty);
}

double WorldGenerator::noise(double x, double y) const {
  double maxAmp = 0;
  double amp = 1;
  double freq = scale_;
  double value = 0;

  for (int i = 0; i < octaves_; ++i) {
    value += open_simplex_noise2(ctx_, x * freq, y * freq) * amp;
    maxAmp += amp;
    amp *= persistence_;
    freq *= 2;
  }

  value /= maxAmp;

  value = value * (high_ - low_) / 2 + (high_ + low_) / 2;

  return value;
}
