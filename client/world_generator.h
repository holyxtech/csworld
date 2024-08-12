#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <memory>
#include <unordered_set>
#include "chunk.h"
#include "open-simplex-noise.h"
#include "section.h"
#include "types.h"
#include "voxel.h"

class WorldGenerator {
public:
  WorldGenerator();
  void fill_chunk(Chunk& chunk, std::unordered_map<Location2D, Section, Location2DHash>& sections);
  bool ready_to_fill(Location& location, const std::unordered_map<Location2D, Section, Location2DHash>& sections) const;

private:
  double noise(double x, double y) const;
  void load_features(Section& section);
  void build_tree(Section& section, int x, int y, int z);

  osn_context* ctx_;
  static constexpr int octaves_ = 4;
  static constexpr double persistence_ = 0.75;
  static constexpr double scale_ = 0.4;
  static constexpr float low_ = 0;
  static constexpr float high_ = 1;
};

#endif