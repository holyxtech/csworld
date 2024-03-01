#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <memory>
#include "chunk.h"
#include "open-simplex-noise.h"
#include "types.h"
#include "section.h"

class WorldGenerator {
public:
  WorldGenerator();
  void fill_chunk(Chunk& chunk, std::unordered_map<Location2D, Section, Location2DHash>& sections) const;
  bool ready_to_fill(Location& location, const std::unordered_map<Location2D, Section, Location2DHash>& sections) const;

private:
  double noise(double x, double y) const;

  osn_context* ctx_;
  static constexpr int octaves_ = 2;
  static constexpr double persistence_ = 0.75;
  static constexpr double scale_ = 0.01;
  static constexpr float low_ = -5;
  static constexpr float high_ = 5;
};

#endif  