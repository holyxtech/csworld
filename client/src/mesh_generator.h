#ifndef MESH_GENERATOR_H
#define MESH_GENERATOR_H

#include <unordered_map>
#include <vector>
#include "region.h"
#include "types.h"
#include "voxel.h"

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
  const std::unordered_map<Location, std::vector<CubeVertex>, LocationHash>& get_meshes() const;
  const std::vector<CubeVertex> get_mesh(const Location& loc) const;
  const std::vector<Vertex> get_irregular_mesh(const Location& loc) const;
  const std::vector<Vertex> get_water_mesh(const Location& loc) const;
  const std::vector<Diff>& get_diffs() const;
  const Location& get_origin() const;
  void clear_diffs();
  static constexpr int defacto_vertices_per_mesh = 80000;
  static constexpr int defacto_vertices_per_irregular_mesh = 4000;
  static constexpr int defacto_vertices_per_water_mesh = 3000;

private:
  void mesh_chunk(const Region& region, const Location& location);
  void mesh_noncube(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel);
  void mesh_water(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel, std::array<Voxel, 6>& adjacent);
  std::array<Voxel, 6> get_adjacent_voxels(const Chunk& chunk, std::array<const Chunk*, 6>& adjacent_chunks, int x, int y, int z) const;

  std::unordered_map<Location, std::vector<CubeVertex>, LocationHash> meshes_;
  std::unordered_map<Location, std::vector<Vertex>, LocationHash> irregular_meshes_;
  std::unordered_map<Location, std::vector<Vertex>, LocationHash> water_meshes_;
  std::vector<Diff> diffs_;

  Location origin_;
  bool origin_set_ = false;
  std::uint32_t random_seed_ = 0;
};

#endif