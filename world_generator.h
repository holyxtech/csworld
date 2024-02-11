#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <memory>
#include "chunk.h"
#include "open-simplex-noise.h"
#include "types.h"

class WorldGenerator {
public:
  WorldGenerator();
  void fill_chunk(Chunk& chunk) const;

private:
  int noise(double x, double y) const;

  osn_context* ctx_;
  static constexpr int octaves_ = 4;
  static constexpr double persistence_ = 0.75;
  static constexpr double scale_ = 0.01;
  static constexpr int low_ = 0;
  static constexpr int high_ = Chunk::sz_y; // map height is undetermined
};

#endif