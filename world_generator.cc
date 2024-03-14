#include "world_generator.h"
#include <cstdlib>
#include <iostream>
#include "poisson_disk_sampling.h"
#include "types.h"

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

void WorldGenerator::insert_into_features(int x, int y, int z, Voxel voxel) {
  auto location = Location{
    static_cast<int>(std::floor(static_cast<float>(x) / Chunk::sz_x)),
    static_cast<int>(std::floor(static_cast<float>(y) / Chunk::sz_y)),
    static_cast<int>(std::floor(static_cast<float>(z) / Chunk::sz_z)),
  };
  auto& location_features = features_[location];

  int xi = ((x % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x;
  int yi = ((y % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y;
  int zi = ((z % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z;

  int idx = Chunk::get_index(
    ((x % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x,
    ((y % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y,
    ((z % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z);

  // these are never removed
  location_features.emplace_back(std::make_pair(idx, voxel));
}

void WorldGenerator::build_tree(int x, int y, int z) {
  insert_into_features(x, y++, z, Voxel::tree_trunk);
  insert_into_features(x, y++, z, Voxel::tree_trunk);
  insert_into_features(x, y++, z, Voxel::tree_trunk);
  insert_into_features(x, y, z, Voxel::tree_trunk);
  insert_into_features(x - 1, y, z, Voxel::leaves);
  insert_into_features(x, y, z - 1, Voxel::leaves);
  insert_into_features(x + 1, y, z, Voxel::leaves);
  insert_into_features(x, y, z + 1, Voxel::leaves);
  insert_into_features(x - 1, y, z - 1, Voxel::leaves);
  insert_into_features(x + 1, y, z + 1, Voxel::leaves);
  insert_into_features(x - 1, y, z + 1, Voxel::leaves);
  insert_into_features(x + 1, y, z - 1, Voxel::leaves);
  insert_into_features(x, y++, z, Voxel::tree_trunk);
  insert_into_features(x, y, z, Voxel::tree_trunk);
  insert_into_features(x - 1, y, z, Voxel::leaves);
  insert_into_features(x, y, z - 1, Voxel::leaves);
  insert_into_features(x + 1, y, z, Voxel::leaves);
  insert_into_features(x, y, z + 1, Voxel::leaves);
  insert_into_features(x - 1, y, z - 1, Voxel::leaves);
  insert_into_features(x + 1, y, z + 1, Voxel::leaves);
  insert_into_features(x - 1, y, z + 1, Voxel::leaves);
  insert_into_features(x + 1, y, z - 1, Voxel::leaves);
  insert_into_features(x, ++y, z, Voxel::leaves);
}

void WorldGenerator::populate(Section& section) {

  float kRadius = 12;
  auto kXMin = std::array<float, 2>{{0.f, 0.f}};
  auto kXMax = std::array<float, 2>{{Section::sz_x, Section::sz_z}};

  auto samples = thinks::PoissonDiskSampling(kRadius, kXMin, kXMax);

  auto& loc = section.get_location();
  for (auto s : samples) {
    auto landcover = section.get_landcover(s[0], s[1]);
    if (landcover != Common::LandCover::trees)
      continue;
    int subsection_elevation = section.get_subsection_elevation(s[0], s[1]);
    int a = loc[0] * Section::sz_x;
    int b = s[0];
    int c = a + s[0];
    int d = a + b;
    build_tree(loc[0] * Section::sz_x + static_cast<int>(s[0]), subsection_elevation + 1, loc[1] * Section::sz_z + static_cast<int>(s[1]));
  }
  //}
}

void WorldGenerator::fill_chunk(Chunk& chunk, std::unordered_map<Location2D, Section, Location2DHash>& sections) {
  auto& location = chunk.get_location();
  std::array<int, 3> arr{-1, 0, 1};
  for (auto x : arr) {
    for (auto z : arr) {
      auto& section = sections.at(Location2D{location[0] + x, location[2] + z});
      if (!section.has_subsection_elevations()) {
        section.compute_subsection_elevations(sections);
        populate(section);
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
      auto n = noise(x + location[0]*Chunk::sz_x, z + location[2]*Chunk::sz_z);
//      std::cout<<n<<std::endl;
      
      if (n > 0.65 && y < (y_global + Chunk::sz_y)) {
        chunk.set_voxel(x, y - y_global, z, Voxel::grass);
      }
    }
  }
  if (empty_subsections < Chunk::sz_x * Chunk::sz_z)
    chunk.set_flag(Chunk::Flags::NONEMPTY);

  auto& features = features_[location];
  for (auto& pair : features) {
    chunk.set_voxel(pair.first, pair.second);
  }
  if (features.size() > 0)
    chunk.set_flag(Chunk::Flags::NONEMPTY);
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
