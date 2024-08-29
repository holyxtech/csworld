#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <memory>
#include <unordered_set>
#include <vector>
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

  std::vector<std::pair<Int3D, Voxel>> build_tree(int x, int y, int z) const;

private:
  struct NoiseGenerator {
    double noise(double x, double y, double shift = 0) {
      x += shift;
      y += shift;
      double maxAmp = 0;
      double amp = 1;
      double freq = scale;
      double value = 0;

      for (int i = 0; i < octaves; ++i) {
        value += open_simplex_noise2(ctx, x * freq, y * freq) * amp;
        maxAmp += amp;
        amp *= persistence;
        freq *= 2;
      }

      value /= maxAmp;

      value = value * (high - low) / 2 + (high + low) / 2;
      return value;
    }

    template <int N>
    std::array<double, N> noises(double x, double y, double shift = 4096) {
      std::array<double, N> arr;
      for (int i = 0; i < N; ++i) {
        arr[i] = noise(x, y, shift * i);
      }
      return arr;
    }

    osn_context* ctx;
    int octaves = 4;
    double persistence = 0.75;
    double scale = 0.4;
    double low = 0;
    double high = 1;
  };

  void load_features(Section& section);

  NoiseGenerator grass_gen_;

  std::vector<bool> tree_roots_;
  static constexpr int tree_root_grid_sz_x = 128;
  static constexpr int tree_root_grid_sz_z = 128;
};

#endif