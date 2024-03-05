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

void WorldGenerator::insert_into_features(int x, int y, int z, Voxel::VoxelType voxel) {
  auto location = Location{
    static_cast<int>(std::floor(static_cast<float>(x) / Chunk::sz_x)),
    static_cast<int>(std::floor(static_cast<float>(y) / Chunk::sz_y)),
    static_cast<int>(std::floor(static_cast<float>(z) / Chunk::sz_z)),
  };
  auto& location_features = features_[location];
  // std::cout << "presumed subsection elevation: " << ((y % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y << std::endl;

  int xi = ((x % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x;
  int yi = ((y % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y;
  int zi = ((z % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z;

  int idx = Chunk::get_index(
    ((x % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x,
    ((y % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y,
    ((z % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z);

  location_features.emplace_back(std::make_pair(idx, voxel));
}

void WorldGenerator::build_tree(int x, int y, int z) {
  // std::cout<<"building tree at: "<<x<<","<<y<<","<<z<<std::endl;

  //

  // just make the thing in global x,y,z coords
  // then with each coord, do a 'insert into local' call
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
  // we've got elevations

  // let's plant trees in features_
  constexpr auto kRadius = 18.F;
  constexpr auto kXMin = std::array<float, 2>{{0.F, 0.F}};
  constexpr auto kXMax = std::array<float, 2>{{Section::sz_x, Section::sz_z}};

  // Samples returned as std::vector<std::array<float, 2>>.
  // Default seed and max sample attempts.
  auto samples = thinks::PoissonDiskSampling(kRadius, kXMin, kXMax);

  auto& loc = section.get_location();
  // std::cout << "populating section: " << loc[0] << "," << loc[1] << std::endl;
  for (auto s : samples) {
    int subsection_elevation = section.get_subsection_elevation(s[0], s[1]);
    int a = loc[0] * Section::sz_x;
    int b = s[0];
    int c = a + s[0];
    int d = a + b;
    build_tree(loc[0] * Section::sz_x + static_cast<int>(s[0]), subsection_elevation + 1, loc[1] * Section::sz_z + static_cast<int>(s[1]));
    //    std::cout << s[0] << "," << s[1] << ", elevation: " << subsection_elevation << std::endl;
  }
}

// Assume only called when all neighbouring sections are present
void WorldGenerator::fill_chunk(Chunk& chunk, std::unordered_map<Location2D, Section, Location2DHash>& sections) {
  auto& location = chunk.get_location();

  // i need more than just the subsection elevations of this section
  // i need the total generations of every subsection in the 3x3 matrix
  // meaning i need 5x5 subsections to exist first

  // to fill chunk you need to know not only what this section contains, but what all surrounding
  // sections contain
  std::array<int, 3> arr{-1, 0, 1};
  for (auto x : arr) {
    for (auto z : arr) {
      auto& section = sections.at(Location2D{location[0] + x, location[2] + z});
      if (!section.has_subsection_elevations()) {
        section.compute_subsection_elevations(sections);

        // when you have the elevations, now you can populate the surface
        populate(section);

        // section loaded order invariance needed....
        // can choose to overwrite neighbouring "higher" (what do i call it?) world features
        // based on location order
        // (todo later)

        // last question... how to choose to unload "higher" features???
        // answer: when the 3x3 matrix of sections surrounding a chunk are removed
        // because then you have to load those sections back in to get the chunk features to generate
      }
    }
  }
  auto& section = sections.at(Location2D{location[0], location[2]});

  Voxel::VoxelType voxel;
  auto landcover = section.get_landcover();
  if (landcover == Common::LandCover::bare) {
    voxel = Voxel::dirt;
  } else if (landcover == Common::LandCover::water) {
    voxel = Voxel::water_full;
  } else {
    voxel = Voxel::dirt;
  }

  int empty_subsections = 0;
  int y_global = location[1] * Chunk::sz_y;
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int x = 0; x < Chunk::sz_x; ++x) {
      int height = section.get_subsection_elevation(x, z);

      if (height < y_global) {
        ++empty_subsections;
        continue;
      }

      for (int y = y_global; y < (y_global + Chunk::sz_y); ++y) {
        if (y <= height)
          chunk.set_voxel(x, y - y_global, z, voxel);
        else
          break;
      }
    }
  }
  if (empty_subsections < Chunk::sz_x * Chunk::sz_z)
    chunk.set_flag(Chunk::Flags::NONEMPTY);

  // std::cout << "Features for chunk at " << location.repr() << std::endl;
  auto& features = features_[location];
  for (auto& pair : features) {
    // std::cout << "at " << pair.first << " put " << pair.second << std::endl;
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
