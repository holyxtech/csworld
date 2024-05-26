#ifndef LOD_MESH_GENERATOR_H
#define LOD_MESH_GENERATOR_H

#include <any>
#include <vector>
#include "lod_loader.h"
#include "types.h"

class LodMeshGenerator {
public:
  struct Diff {
    struct CreationData {
      LodLevel level;
    };
    enum Kind {
      creation,
    };
    Kind kind;
    Location location;
    std::any data;

    template <typename T>
    Diff(Kind k, Location loc, const T& obj) : kind(k), location(loc), data(obj) {}
  };

  void consume_lod_loader(LodLoader& lod_loader);
  void clear_diffs();
  const std::vector<Diff>& get_diffs() const;
  template <LodLevel level>
  const std::vector<LodVertex>& get_mesh(const Location& loc) const;

  static constexpr int defacto_vertices_per_lod1_mesh = 1000;
  
private:
  struct MeshPack {
    std::vector<LodVertex> l1;
    std::vector<LodVertex> l2;

    template <LodLevel level>
    const std::vector<LodVertex>& get() const;
    template <LodLevel level>
    std::vector<LodVertex>& get();
  };

  template <LodLevel level>
  void mesh_chunk(LodLoader& lod_loader, const Location& location);
  template <LodLevel level>
  std::array<Voxel, 6> get_adjacent_voxels(
    const ChunkLod<level>& lod, std::array<const ChunkLod<level>*, 6>& adjacent_lods, int x, int y, int z);

  std::unordered_map<Location, MeshPack, LocationHash> meshes_;
  std::vector<Diff> diffs_;
};

#endif