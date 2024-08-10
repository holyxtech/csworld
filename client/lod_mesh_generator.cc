#include "lod_mesh_generator.h"
#include "mesh_utils.h"

template <LodLevel level>
std::array<Voxel, 6> LodMeshGenerator::get_adjacent_voxels(
  const ChunkLod<level>& lod, std::array<const ChunkLod<level>*, 6>& adjacent_lods, int x, int y, int z) {
  std::array<Voxel, 6> adjacent;
  if (x > 0)
    adjacent[nx] = lod.get_voxel(x - 1, y, z);
  else
    adjacent[nx] = adjacent_lods[nx]->get_voxel(ChunkLod<level>::sz_x - 1, y, z);
  if (x < ChunkLod<level>::sz_x - 1)
    adjacent[px] = lod.get_voxel(x + 1, y, z);
  else
    adjacent[px] = adjacent_lods[px]->get_voxel(0, y, z);
  if (y > 0)
    adjacent[ny] = lod.get_voxel(x, y - 1, z);
  else
    adjacent[ny] = adjacent_lods[ny]->get_voxel(x, ChunkLod<level>::sz_y - 1, z);
  if (y < ChunkLod<level>::sz_y - 1)
    adjacent[py] = lod.get_voxel(x, y + 1, z);
  else
    adjacent[py] = adjacent_lods[py]->get_voxel(x, 0, z);
  if (z > 0)
    adjacent[nz] = lod.get_voxel(x, y, z - 1);
  else
    adjacent[nz] = adjacent_lods[nz]->get_voxel(x, y, ChunkLod<level>::sz_z - 1);
  if (z < ChunkLod<level>::sz_z - 1)
    adjacent[pz] = lod.get_voxel(x, y, z + 1);
  else
    adjacent[pz] = adjacent_lods[pz]->get_voxel(x, y, 0);
  return adjacent;
}

template <LodLevel level>
void LodMeshGenerator::mesh_chunk(LodLoader& lod_loader, const Location& location) {
  auto& lod = lod_loader.get_lod<level>(location);

  auto adjacent_lods = lod_loader.get_adjacent_lods<level>(location);
  auto& mesh = meshes_[location].get<level>();
  for (int z = 0; z < ChunkLod<level>::sz_z; ++z) {
    for (int y = 0; y < ChunkLod<level>::sz_y; ++y) {
      for (int x = 0; x < ChunkLod<level>::sz_x; ++x) {
        auto voxel = lod.get_voxel(x, y, z);
        if (!vops::is_cube(voxel))
          continue;
        auto adjacent = get_adjacent_voxels<level>(lod, adjacent_lods, x, y, z);
        auto voxel_textures = MeshUtils::get_textures(voxel, adjacent);
        auto& textures = reinterpret_cast<std::array<int, 6>&>(voxel_textures);

        if (!vops::is_opaque(adjacent[nx])) {
          mesh.emplace_back(LodVertex(x, y, z, nx, br, textures[nx]));
          mesh.emplace_back(LodVertex(x, y + 1, z + 1, nx, tl, textures[nx]));
          mesh.emplace_back(LodVertex(x, y + 1, z, nx, tr, textures[nx]));
          mesh.emplace_back(LodVertex(x, y, z, nx, br, textures[nx]));
          mesh.emplace_back(LodVertex(x, y, z + 1, nx, bl, textures[nx]));
          mesh.emplace_back(LodVertex(x, y + 1, z + 1, nx, tl, textures[nx]));
        }
        if (!vops::is_opaque(adjacent[px])) {
          mesh.emplace_back(LodVertex(x + 1, y, z, px, bl, textures[px]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z, px, tl, textures[px]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z + 1, px, tr, textures[px]));
          mesh.emplace_back(LodVertex(x + 1, y, z, px, bl, textures[px]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z + 1, px, tr, textures[px]));
          mesh.emplace_back(LodVertex(x + 1, y, z + 1, px, br, textures[px]));
        }
        if (!vops::is_opaque(adjacent[ny])) {
          mesh.emplace_back(LodVertex(x, y, z, ny, br, textures[ny]));
          mesh.emplace_back(LodVertex(x + 1, y, z + 1, ny, tr, textures[ny]));
          mesh.emplace_back(LodVertex(x, y, z + 1, ny, tl, textures[ny]));
          mesh.emplace_back(LodVertex(x, y, z, ny, br, textures[ny]));
          mesh.emplace_back(LodVertex(x + 1, y, z, ny, tl, textures[ny]));
          mesh.emplace_back(LodVertex(x + 1, y, z + 1, ny, bl, textures[ny]));
        }
        if (!vops::is_opaque(adjacent[py])) {
          mesh.emplace_back(LodVertex(x, y + 1, z, py, br, textures[py]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z + 1, py, tl, textures[py]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z, py, tr, textures[py]));
          mesh.emplace_back(LodVertex(x, y + 1, z, py, br, textures[py]));
          mesh.emplace_back(LodVertex(x, y + 1, z + 1, py, bl, textures[py]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z + 1, py, tl, textures[py]));
        }
        if (!vops::is_opaque(adjacent[nz])) {
          mesh.emplace_back(LodVertex(x, y, z, nz, bl, textures[nz]));
          mesh.emplace_back(LodVertex(x, y + 1, z, nz, tl, textures[nz]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z, nz, tr, textures[nz]));
          mesh.emplace_back(LodVertex(x, y, z, nz, bl, textures[nz]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z, nz, tr, textures[nz]));
          mesh.emplace_back(LodVertex(x + 1, y, z, nz, br, textures[nz]));
        }
        if (!vops::is_opaque(adjacent[pz])) {
          mesh.emplace_back(LodVertex(x, y, z + 1, pz, br, textures[pz]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z + 1, pz, tl, textures[pz]));
          mesh.emplace_back(LodVertex(x, y + 1, z + 1, pz, tr, textures[pz]));
          mesh.emplace_back(LodVertex(x, y, z + 1, pz, br, textures[pz]));
          mesh.emplace_back(LodVertex(x + 1, y, z + 1, pz, bl, textures[pz]));
          mesh.emplace_back(LodVertex(x + 1, y + 1, z + 1, pz, tl, textures[pz]));
        }
      }
    }
  }
}

void LodMeshGenerator::consume_lod_loader(LodLoader& lod_loader) {
  auto& diffs = lod_loader.get_diffs();
  for (auto& diff : diffs) {
    if (diff.kind == LodLoader::Diff::creation) {
      auto& loc = diff.location;
      mesh_chunk<LodLevel::lod1>(lod_loader, loc);
      diffs_.emplace_back(Diff{Diff::creation, loc, Diff::CreationData{LodLevel::lod1}});
    }
  }
  lod_loader.clear_diffs();
}

void LodMeshGenerator::clear_diffs() {
  diffs_.clear();
}
const std::vector<LodMeshGenerator::Diff>& LodMeshGenerator::get_diffs() const {
  return diffs_;
}

template <LodLevel level>
const std::vector<LodVertex>& LodMeshGenerator::get_mesh(const Location& loc) const {
  return meshes_.at(loc).get<level>();
}

template const std::vector<LodVertex>& LodMeshGenerator::get_mesh<LodLevel::lod1>(const Location& loc) const;
template const std::vector<LodVertex>& LodMeshGenerator::get_mesh<LodLevel::lod2>(const Location& loc) const;

template <LodLevel level>
std::vector<LodVertex>& LodMeshGenerator::MeshPack::get() {
  if constexpr (level == LodLevel::lod1)
    return l1;
  else if constexpr (level == LodLevel::lod2)
    return l2;
}

template <LodLevel level>
const std::vector<LodVertex>& LodMeshGenerator::MeshPack::get() const {
  return const_cast<MeshPack*>(this)->template get<level>();
}