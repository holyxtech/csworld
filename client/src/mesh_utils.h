#include <array>
#include "voxel.h"

namespace MeshUtils {
  std::array<VT, 6> get_textures(Voxel voxel, std::array<Voxel, 6>& adjacent);
}