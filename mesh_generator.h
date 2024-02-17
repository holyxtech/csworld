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
      water
    };
    Location location;  
    Kind kind;
  };
  void consume_region(Region& region);
  const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& get_meshes() const;
  const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& get_water_meshes() const;
  const std::vector<Diff>& get_diffs() const;
  void clear_diffs();
  static constexpr int default_max_vertices = 150000;
  static constexpr int default_max_water_vertices = 30000;
private:
  void mesh_chunk(const Chunk& chunk);
  void mesh_greedy(const Chunk& chunk);
  void mesh_water(const Chunk& chunk);
  std::array<Voxel::VoxelType, 6> get_adjacent_voxels(const Chunk& chunk, int x, int y, int z) const;
  void fill_sides(std::vector<Vertex>& mesh, glm::vec3& position, std::array<Voxel::VoxelType, 6>& adjacent, std::array<int, 6>& layers, Voxel::VoxelType TYPE_UPPER_BOUND);

  std::unordered_map<Location, std::vector<Vertex>, LocationHash> meshes_;
  std::unordered_map<Location, std::vector<Vertex>, LocationHash> water_meshes_;
  std::vector<Diff> diffs_;
};

#endif