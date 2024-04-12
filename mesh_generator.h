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
      deletion,
      origin
    };
    Location location;
    Kind kind;
  };
  MeshGenerator();
  void consume_region(Region& region);
  const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& get_meshes() const;
  const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& get_water_meshes() const;
  const std::vector<Diff>& get_diffs() const;
  void clear_diffs();
  static constexpr int default_max_vertices = 33000;
  static constexpr int default_max_water_vertices = 10000;

private:
  void mesh_chunk(Region& region, const Location& location);
  void mesh_water(Region& region, const Location& location);
  std::array<Voxel, 6> get_adjacent_voxels(Chunk& chunk, std::array<Chunk*,6>& adjacent_chunks, int x, int y, int z) const;
  void fill_sides(
    std::vector<Vertex>& mesh, glm::vec3& position, std::array<Voxel, 6>& adjacent,
    std::array<VoxelTexture, 6>& layers, Voxel TYPE_UPPER_BOUND, std::array<float, 6>& lighting);
  void mesh_noncube(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel);
  std::array<float, 6> get_lighting(Chunk& chunk, std::array<Chunk*,6>& adjacent_chunks, int x, int y, int z) const;

  std::unordered_map<Location, std::vector<Vertex>, LocationHash> meshes_;
  std::unordered_map<Location, std::vector<Vertex>, LocationHash> water_meshes_;
  std::vector<Diff> diffs_;
  std::array<float, Chunk::max_lighting+1> lighting_levels_;

  int x_lighting_reduction = 3;
  Location origin_;
  bool origin_set_ = false;
  std::uint32_t random_seed_ = 0;
};

#endif