#include "mesh_utils.h"
#include "types.h"

namespace MeshUtils {
  std::array<VT, 6> get_textures(Voxel voxel, std::array<Voxel, 6>& adjacent) {
    std::array<VT, 6> textures;
    switch (voxel) {
    case Voxel::dirt:
      if (adjacent[py] < Voxel::OPAQUE_LOWER) {
        textures[nx] = CubeTexture::grass;
        textures[px] = CubeTexture::grass;
        textures[nz] = CubeTexture::grass;
        textures[pz] = CubeTexture::grass;
      } else {
        textures[nx] = CubeTexture::dirt;
        textures[px] = CubeTexture::dirt;
        textures[nz] = CubeTexture::dirt;
        textures[pz] = CubeTexture::dirt;
      }
      if (adjacent[py] == Voxel::water_full) {
        textures[py] = CubeTexture::grass;
      } else {
        textures[py] = CubeTexture::grass;
      }
      textures[ny] = CubeTexture::dirt;
      break;
    case Voxel::sand:
      textures.fill(CubeTexture::sand);
      break;
    case Voxel::tree_trunk:
      textures.fill(CubeTexture::tree_trunk);
      break;
    case Voxel::leaves:
      textures.fill(CubeTexture::leaves);
      break;
    case Voxel::sandstone:
      textures.fill(CubeTexture::sandstone);
      break;
    case Voxel::stone:
      textures.fill(CubeTexture::stone);
      break;
    }
    return textures;
  }

} // namespace MeshUtils
