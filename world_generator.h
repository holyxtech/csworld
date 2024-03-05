#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <memory>
#include "chunk.h"
#include "open-simplex-noise.h"
#include "section.h"
#include "types.h"

class WorldGenerator {
public:
  WorldGenerator();
  void fill_chunk(Chunk& chunk, std::unordered_map<Location2D, Section, Location2DHash>& sections);
  bool ready_to_fill(Location& location, const std::unordered_map<Location2D, Section, Location2DHash>& sections) const;

private:
  double noise(double x, double y) const;
  void populate(Section& section);
  void build_tree(int x, int y, int z);
  void insert_into_features(int x, int y, int z, Voxel::VoxelType voxel);

  std::unordered_map<Location, std::vector<std::pair<int, Voxel::VoxelType>>, LocationHash> features_;

  osn_context* ctx_;
  static constexpr int octaves_ = 2;
  static constexpr double persistence_ = 0.75;
  static constexpr double scale_ = 0.01;
  static constexpr float low_ = -5;
  static constexpr float high_ = 5;
};

#endif