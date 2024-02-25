#include "world_generator.h"
#include <cstdlib>
#include <iostream>
#include "types.h"

WorldGenerator::WorldGenerator() {
  open_simplex_noise(7, &ctx_);
}

bool WorldGenerator::ready_to_fill(Location& location, const std::unordered_map<Location2D, Section, Location2DHash>& sections) const {
  std::array<int, 3> arr{-1, 0, 1};
  for (auto x : arr) {
    for (auto z : arr) {
      if (!sections.contains(Location2D{location[0] + x, location[2] + z}))
        return false;
    }
  }
  return true;
}

// Assume only called when all neighbouring sections are present
void WorldGenerator::fill_chunk(Chunk& chunk, std::unordered_map<Location2D, Section, Location2DHash>& sections) const {
  auto& location = chunk.get_location();

  // get the terrain heights of the x-z in chunk section, if doesn't exist create it
  auto& section = sections.at(Location2D{location[0], location[2]});
  if (!section.has_subsection_elevations()) {
    section.compute_subsection_elevations(sections);
  }

  // transform the local chunk y coordinates to global coordinates and compare against terrain height
  int y_global = location[1] * Chunk::sz_y;
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int x = 0; x < Chunk::sz_x; ++x) {
      // what is the terrain height at this point in the section?

      // global coords...
      int height = section.get_subsection_elevation(x, z);
      /* int n = noise(location[0] * Chunk::sz_x + x, location[2] * Chunk::sz_z + z);
      height += n; */
      //std::cout << n << std::endl;

      // global y at chunk bottom

      // while y < height, add solid
      for (int y = y_global; y < (y_global + Chunk::sz_y); ++y) {
        if (y < height)
          chunk.set_voxel(x, y - y_global, z, Voxel::dirt);
        else
          chunk.set_voxel(x, y - y_global, z, Voxel::empty);
      }
    }
  }

  /*   auto position = Location{location[0] * Chunk::sz_x, location[1] * Chunk::sz_y, location[2] * Chunk::sz_z};
    for (int z = 0; z < Chunk::sz_z; ++z) {
      for (int x = 0; x < Chunk::sz_x; ++x) {
        double voxel_x = position[0] + x;
        double voxel_z = position[2] + z;
        int height = noise(voxel_x, voxel_z);
        int y = 0;
        for (; y < height; ++y) {
          chunk.set_voxel(x, y, z, Voxel::dirt);
        }
        for (; y < 25; ++y) {
          chunk.set_voxel(x, y, z, Voxel::sand);
        }
        for (; y < 30; ++y) {
          chunk.set_voxel(x, y, z, Voxel::water_full);
        }
      }
    } */
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
