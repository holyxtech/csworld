#include "world_generator.h"
#include <cstdlib>
#include <iostream>
#include <cy/cyPoint.h>
#include <cy/cySampleElim.h>
#include "cs_math.h"
#include "region.h"

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
  open_simplex_noise(7, &grass_gen_.ctx);

  tree_roots_.resize(tree_root_grid_sz_x * tree_root_grid_sz_z, false);
  cy::WeightedSampleElimination<cy::Point2d, double, 2> wse;
  wse.SetParamBeta(0.0);
  wse.SetBoundsMin(cy::Point2d{0, 0});
  wse.SetBoundsMax(cy::Point2d{tree_root_grid_sz_x - 1, tree_root_grid_sz_z - 1});
  wse.SetTiling(true);
  std::vector<cy::Point2d> input_points;
  for (double z = 0; z < tree_root_grid_sz_z; ++z) {
    for (double x = 0; x < tree_root_grid_sz_x; ++x) {
      input_points.push_back(cy::Point2d{x, z});
    }
  }
  int sparsity = 50;
  std::vector<cy::Point2d> output_points(input_points.size() / sparsity);
  wse.Eliminate(
    input_points.data(), input_points.size(),
    output_points.data(), output_points.size(), true);
  for (auto& p : output_points) {
    int x = static_cast<int>(p.x);
    int z = static_cast<int>(p.y);
    tree_roots_[x + tree_root_grid_sz_x * z] = true;
  }
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
  int tree_height = common::random_int(5, 8);
  int height_without_leaves;
  if (tree_height >= 7) {
    height_without_leaves = common::random_int(3, 4);
  } else {
    height_without_leaves = common::random_int(2, 3);
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

  int sec_x_offset = cs_math::mod(loc[0], (tree_root_grid_sz_x / Section::sz_x)) * Section::sz_x;
  int sec_z_offset = cs_math::mod(loc[1], (tree_root_grid_sz_z / Section::sz_z)) * Section::sz_z;
  for (int z = 0; z < Section::sz_z; ++z) {
    for (int x = 0; x < Section::sz_x; ++x) {
      auto landcover = section.get_landcover(x, z);
      int subsection_elevation = section.get_subsection_elevation(x, z);
      int x_global = loc[0] * Section::sz_x + static_cast<int>(x);
      int z_global = loc[1] * Section::sz_z + static_cast<int>(z);
      if (landcover == common::LandCover::trees) {
        if (tree_roots_[(x + sec_x_offset) + tree_root_grid_sz_x * (z + sec_z_offset)]) {
          auto tree = build_tree(x_global, subsection_elevation + 1, z_global);
          for (auto& [coord, voxel] : tree)
            section.insert_into_features(coord[0], coord[1], coord[2], voxel);
        } else {
          auto [n1, n2, n3] = grass_gen_.noises<3>(x, z);
          if (n1 > 0.6)
            section.insert_into_features(x_global, subsection_elevation + 1, z_global, Voxel::grass);
          else if (n2 > 0.7)
            section.insert_into_features(x_global, subsection_elevation + 1, z_global, Voxel::sunflower);
          else if (n3 > 0.7)
            section.insert_into_features(x_global, subsection_elevation + 1, z_global, Voxel::roses);
        }
      } else if (
        landcover == common::LandCover::grass ||
        landcover == common::LandCover::shrubs) {
        auto n = grass_gen_.noise(x, z);
        if (n > 0.65)
          section.insert_into_features(x_global, subsection_elevation + 1, z_global, Voxel::grass);
      }
    }
  }
}

void WorldGenerator::fill_chunk(Chunk& chunk, std::unordered_map<Location2D, Section, Location2DHash>& sections) {
  auto& location = chunk.get_location();

  for (auto x : {-1, 0, 1}) {
    for (auto z : {-1, 0, 1}) {
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
      if (landcover == common::LandCover::bare) {
        voxel = Voxel::stone;
      } else if (landcover == common::LandCover::water) {
        voxel = Voxel::water_full;
      } else {
        voxel = Voxel::dirt;
      }

      int y = y_global;
      for (; y < (y_global + Chunk::sz_y) && y <= height; ++y) {
        chunk.set_voxel(x, y - y_global, z, voxel);
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
