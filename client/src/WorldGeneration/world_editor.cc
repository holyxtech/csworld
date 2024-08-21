#include "world_editor.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <unordered_map>
#include <cyPoint.h>
#include <cySampleElim.h>
#include "sim.h"
#include "common.h"

namespace {
  glm::dvec3 down_dir = glm::dvec3{0, -1, 0};
  glm::dvec3 up_dir = glm::dvec3{0, 1, 0};
} // namespace

int WorldEditor::max_dropoff_distance = 10;
int WorldEditor::max_lift = 10;
int WorldEditor::brush_radius = 5;
// Constant multiplied in Gaussian (how fast it acts)
float WorldEditor::brush_strength = 0.2;
// Sigma of Gaussian
float WorldEditor::brush_spread = 4;

WorldEditor::WorldEditor(Sim& sim) : sim_(sim) {
  heightmap_.reserve(heightmap_sz);
}

float WorldEditor::gaussian_falloff(float distance_squared) const {
  return brush_strength * std::exp(-(distance_squared) / (2 * brush_spread * brush_spread));
}

/* 1. raycast to find first EARTHEN voxel
   2. if new coord, reset + new origin
   3. determine heights of earthen voxels
      in NxN radius around new coord down to some minimum (determined by brush falloff?),
      and up to a maximum of the new coord height
   4. compute heightmap increase in NxN radius
   5. for each increase, if integer tick, raise column at that location
   6. maintain unordered_map of locations that have been modified, as well as adjacdent that need to be updated
   7. get region to generate diffs

   Notes:
   - falloff can be clamped so e.g all voxels in NxM region of brush tip are treated the same
   - need to generate update in some form that can be undone
*/
void WorldEditor::raise(const glm::dvec3& pos, const glm::dvec3& dir) {
  /*   Int3D coord;
    Voxel voxel_at_brush
    _tip;
    if (!region_.get_first_of_kind(pos, dir, 500, coord, voxel_at_brush_tip, vops::is_earthen))
      return;

    brush_tip_local_ = Int2D{heightmap_sz_x / 2, heightmap_sz_z / 2};

    if (!((coord[0]-brush_tip_local_[0]) == heightmap_origin_[0] &&
          (coord[2]-brush_tip_local_[1]) == heightmap_origin_[1])) {
      reset();
      heightmap_origin_ = Int2D{coord[0] -  brush_tip_local_[1], coord[2] - brush_tip_local_[1]};

    }

    std::unordered_set<Location, LocationHash> dirty;

    for (int z = -brush_radius; z <= brush_radius; ++z) {
      for (int x = -brush_radius; x <= brush_radius; ++x) {
        glm::dvec3 adjacent_pos_center{coord[0] + x + 0.5, coord[1] - 0.5, coord[2] + z + 0.5};
        Int3D adjacent_coord;
        Voxel adjacent_voxel;
        bool found = region_.get_first_of_kind(
          adjacent_pos_center, down_dir, max_dropoff_distance + 1, adjacent_coord, adjacent_voxel, vops::is_earthen);
        if (!found)
          continue;
        int idx = (x + brush_tip_local_[0]) + heightmap_sz_x * (z + brush_tip_local_[1]);
        float distance_squared = x * x + z * z;
        float falloff = gaussian_falloff(distance_squared);
        float previous_height = heightmap_[idx];
        heightmap_[idx] += falloff;
        float floor_ph = std::floor(previous_height);
        float floor_phf = std::floor(previous_height + falloff);
        if (std::floor(previous_height) != std::floor(previous_height + falloff)) {

          std::vector<Voxel> voxels;
          bool can_lift = region_.get_until_kind(adjacent_pos_center, up_dir, max_lift + 1, voxels, vops::is_empty);
          if (!can_lift)
            continue;
          Int3D current_coord = Int3D{adjacent_coord[0], adjacent_coord[1] + 1, adjacent_coord[2]};
          for (auto voxel : voxels) {
            auto loc = Region::location_from_global_coord(current_coord);
            dirty.insert(loc);
            auto& chunk = region_.get_chunk(loc);
            int x_local = ((current_coord[0] % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x;
            int y_local = ((current_coord[1] % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y;
            int z_local = ((current_coord[2] % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z;
            chunk.set_voxel(x_local, y_local, z_local, voxel);
            if (x_local == 0) {
              dirty.insert(Location{loc[0] - 1, loc[1], loc[2]});
            } else if (x_local == Chunk::sz_x - 1) {
              dirty.insert(Location{loc[0] + 1, loc[1], loc[2]});
            }
            if (y_local == 0) {
              dirty.insert(Location{loc[0], loc[1] - 1, loc[2]});
            } else if (y_local == Chunk::sz_y - 1) {
              dirty.insert(Location{loc[0], loc[1] + 1, loc[2]});
            }
            if (z_local == 0) {
              dirty.insert(Location{loc[0], loc[1], loc[2] - 1});
            } else if (z_local == Chunk::sz_z - 1) {
              dirty.insert(Location{loc[0], loc[1], loc[2] + 1});
            }

            current_coord[1] += 1;
          }
        }
      }
    }
    for (auto& loc : dirty) {
       region_.signal_chunk_update(loc);
    } */
}

void WorldEditor::reset() {
  //  heightmap_.fill(0.);
}

void WorldEditor::generate(const std::unordered_set<Int3D, LocationHash>& surface) {
  if (surface.size() == 0)
    return;
  auto& world_generator = sim_.get_world_generator();
  auto& region = sim_.get_region();

  std::unordered_map<Int2D, int, Location2DHash> recover;
  std::vector<cy::Point2d> rawPoints;
  int min_x = std::numeric_limits<int>::max();
  int min_z = std::numeric_limits<int>::max();
  int max_x = std::numeric_limits<int>::lowest();
  int max_z = std::numeric_limits<int>::lowest();
  for (auto& coord : surface) {

    if (coord[0] < min_x)
      min_x = coord[0];
    if (coord[2] < min_z)
      min_z = coord[2];
    if (coord[0] > max_x)
      max_x = coord[0];
    if (coord[2] > max_z)
      max_z = coord[2];
  }

  for (auto& coord : surface) {
    rawPoints.emplace_back(coord[0] - min_x, coord[2] - min_z);
    recover.insert({Int2D{coord[0], coord[2]}, coord[1]});
  }

  int sparsity = 25;
  std::vector<cy::Point2d> outputPoints(surface.size() / sparsity);

  double x_bounds = (max_x - min_x);
  double z_bounds = (max_z - min_z);
  double bounds = std::max(x_bounds, z_bounds);
  cy::WeightedSampleElimination<cy::Point2d, double, 2> wse;
  wse.SetBoundsMin(cy::Point2d{0, 0});
  wse.SetBoundsMax(cy::Point2d{static_cast<double>(max_x - min_x), static_cast<double>(max_z - min_z)});
  wse.SetParamBeta(0.0);
  wse.Eliminate(
    rawPoints.data(), rawPoints.size(),
    outputPoints.data(), outputPoints.size(), true);

  std::unordered_set<Location, LocationHash> dirty;

  auto try_to_place_voxel_at_coord = [&region, &dirty](const Int3D& coord, Voxel voxel) {
    auto loc = Region::location_from_global_coord(coord);
    auto local_coord = Chunk::to_local(coord);
    int idx = Chunk::get_index(local_coord);
    bool success = region.set_voxel_if_possible(loc, idx, voxel);
    if (success) {
      dirty.insert(loc);
      if (local_coord[0] == 0) {
        dirty.insert(Location{loc[0] - 1, loc[1], loc[2]});
      } else if (local_coord[0] == Chunk::sz_x - 1) {
        dirty.insert(Location{loc[0] + 1, loc[1], loc[2]});
      }
      if (local_coord[1] == 0) {
        dirty.insert(Location{loc[0], loc[1] - 1, loc[2]});
      } else if (local_coord[1] == Chunk::sz_y - 1) {
        dirty.insert(Location{loc[0], loc[1] + 1, loc[2]});
      }
      if (local_coord[2] == 0) {
        dirty.insert(Location{loc[0], loc[1], loc[2] - 1});
      } else if (local_coord[2] == Chunk::sz_z - 1) {
        dirty.insert(Location{loc[0], loc[1], loc[2] + 1});
      }
    }
  };

  for (auto& p : outputPoints) {
    int x = static_cast<int>(p.x) + min_x;
    int z = static_cast<int>(p.y) + min_z;
    int y = recover.at(Int2D{x, z}) + 1;
    auto tree = world_generator.build_tree(x, y, z);
    for (auto& [coord, voxel] : tree) {
      try_to_place_voxel_at_coord(coord, voxel);
    }
  }

  // flowers and stuff...
  std::unordered_set<Int2D, Location2DHash> left_over;
  for (auto& p : rawPoints) {
    left_over.insert(Int2D{static_cast<int>(p.x), static_cast<int>(p.y)});
  }
  for (auto& p : outputPoints) {
    left_over.erase(Int2D{static_cast<int>(p.x), static_cast<int>(p.y)});
  }
  rawPoints.clear();
  for (auto [x, z] : left_over) {
    rawPoints.emplace_back(x, z);
  }
  sparsity = 3;
  outputPoints.resize(rawPoints.size() / sparsity);
  wse.SetBoundsMax(cy::Point2d{static_cast<double>(max_x - min_x), static_cast<double>(max_z - min_z)});
  wse.Eliminate(
    rawPoints.data(), rawPoints.size(),
    outputPoints.data(), outputPoints.size());
  for (auto& p : outputPoints) {
    int x = static_cast<int>(p.x) + min_x;
    int z = static_cast<int>(p.y) + min_z;
    int y = recover.at(Int2D{x, z}) + 1;
    Voxel voxel = Voxel::grass;
    float probability = Common::random_probability();
    if (probability > 0.66) {
      voxel = Voxel::sunflower;
    } else if (probability > 0.33) {
      voxel = Voxel::roses;
    }
    try_to_place_voxel_at_coord(Int3D{x, y, z}, voxel);
  }

  for (auto& loc : dirty) {
    region.signal_chunk_update(loc);
  }
}
