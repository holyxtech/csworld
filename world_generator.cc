#include "world_generator.h"
#include <cstdlib>
#include <iostream>
#include "types.h"

WorldGenerator::WorldGenerator() {
  open_simplex_noise(7, &ctx_);
}

void WorldGenerator::fill_chunk(Chunk& chunk) const {
  auto& location = chunk.get_location();
  auto position = Location{location[0] * Chunk::sz_x, location[1] * Chunk::sz_y, location[2] * Chunk::sz_z};
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int x = 0; x < Chunk::sz_x; ++x) {
      double voxel_x = position[0] + x;
      double voxel_z = position[2] + z;
      int height = noise(voxel_x, voxel_z);
      for (int y = 0; y < height; ++y) {
        chunk.set_voxel(x, y, z, Voxel::dirt);
      }
    }
  }
}

int WorldGenerator::noise(double x, double y) const {
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
