#include "world_editor.h"
#include <cmath>

int WorldEditor::max_dropoff_distance = 10;
int WorldEditor::brush_size = 10;

WorldEditor::WorldEditor(Region& region) : region_(region) {
  heightmap_.fill(0.f);
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
*/
void WorldEditor::raise(glm::dvec3& pos, glm::dvec3& dir) {

  auto visited = region_.raycast(pos, dir, 500);
  Int3D coord;
  Voxel voxel;
  if (!region_.get_first_of_kind(visited, coord, voxel, vops::is_earthen))
    return;

  if (!(coord[0] == heightmap_origin_[0] &&
        coord[2] == heightmap_origin_[1])) {
    reset();
    heightmap_origin_ = Int2D{coord[0] - heightmap_sz_x / 2, coord[2] - heightmap_sz_z / 2};
  }

  for (int z = 0; z < heightmap_sz_z; ++z) {
    for (int x = 0; x < heightmap_sz_x; ++x) {
    }
  }
}

void WorldEditor::reset() {
  heightmap_.fill(0.);
}