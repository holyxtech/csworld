#ifndef MESH_GENERATOR_H
#define MESH_GENERATOR_H

#include <unordered_map>
#include <vector>
#include "region.h"
#include "types.h"

class MeshGenerator {
public:
  struct Diff {
    enum Kind {
      creation,
      deletion
    };
    Location location;
    Kind kind;
  };
  void consume_region(Region& region);
  const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& get_meshes() const;
  const std::vector<Diff>& get_diffs() const;
  void clear_diffs();

private:
  void mesh_chunk(const Chunk& chunk);
  std::array<Voxel::VoxelType, 6> get_adjacent_voxels(const Chunk& chunk, int x, int y, int z) const;
  std::array<int, 6> tex_layers(Voxel::VoxelType voxel, std::array<Voxel::VoxelType, 6> adjacent) const;

  std::unordered_map<Location, std::vector<Vertex>, LocationHash> meshes_;
  std::vector<Diff> diffs_;
};

#endif