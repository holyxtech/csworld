#include "world_editor.h"
#include <cmath>

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

WorldEditor::WorldEditor(Region& region) : region_(region) {
  heightmap_.fill(0.f);
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
  Voxel voxel_at_brush_tip;
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
  heightmap_.fill(0.);
}