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
  static constexpr int defacto_vertices_per_mesh = 40000;
  static constexpr int defacto_vertices_per_irregular_mesh = 1000;
  static constexpr int defacto_vertices_per_water_mesh = 3000;

private:
  enum class Axis {
    x,
    y,
    z
  };

  void mesh_chunk(Region& region, const Location& location);
  void mesh_noncube(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel, float lighting);
  void mesh_water(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel, std::array<Voxel, 6>& adjacent);
  std::array<float, 4> get_lighting(Region& region, Int3D coord, Axis axis) const;
  std::array<Voxel, 6> get_adjacent_voxels(Chunk& chunk, std::array<Chunk*, 6>& adjacent_chunks, int x, int y, int z) const;

  std::unordered_map<Location, std::vector<CubeVertex>, LocationHash> meshes_;
  std::unordered_map<Location, std::vector<Vertex>, LocationHash> irregular_meshes_;
  std::unordered_map<Location, std::vector<Vertex>, LocationHash> water_meshes_;
  std::vector<Diff> diffs_;
  std::array<float, Chunk::max_lighting + 1> lighting_levels = {
    0.03518437208883203,
    0.043980465111040035,
    0.054975581388800036,
    0.06871947673600004,
    0.08589934592000005,
    0.10737418240000006,
    0.13421772800000006,
    0.1677721600000001,
    0.20971520000000007,
    0.2621440000000001,
    0.3276800000000001,
    0.4096000000000001,
    0.5120000000000001,
    0.6400000000000001,
    0.8,
    1.0};

  static constexpr std::array<std::array<Int3D, 9>, 3> three_by_three_grid_vectors = {
    std::array<Int3D, 9>{
      Int3D{0, -1, -1},
      Int3D{0, 0, -1},
      Int3D{0, 1, -1},

      Int3D{0, -1, 0},
      Int3D{0, 0, 0},
      Int3D{0, 1, 0},

      Int3D{0, -1, 1},
      Int3D{0, 0, 1},
      Int3D{0, 1, 1}},

    std::array<Int3D, 9>{
      Int3D{-1, 0, -1},
      Int3D{0, 0, -1},
      Int3D{1, 0, -1},

      Int3D{-1, 0, 0},
      Int3D{0, 0, 0},
      Int3D{1, 0, 0},

      Int3D{-1, 0, 1},
      Int3D{0, 0, 1},
      Int3D{1, 0, 1}},

    std::array<Int3D, 9>{
      Int3D{-1, -1, 0},
      Int3D{0, -1, 0},
      Int3D{1, -1, 0},

      Int3D{-1, 0, 0},
      Int3D{0, 0, 0},
      Int3D{1, 0, 0},

      Int3D{-1, 1, 0},
      Int3D{0, 1, 0},
      Int3D{1, 1, 0},
    }

  };

  static constexpr int z_lighting_reduction = 3;
  Location origin_;
  bool origin_set_ = false;
  std::uint32_t random_seed_ = 0;
};

#endif