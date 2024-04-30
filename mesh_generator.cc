#include "mesh_generator.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

MeshGenerator::MeshGenerator() {
  lighting_levels_ = {
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

  three_by_three_grid_vectors_[static_cast<int>(Axis::x)] = {
    Int3D{0, -1, -1},
    Int3D{0, 0, -1},
    Int3D{0, 1, -1},

    Int3D{0, -1, 0},
    Int3D{0, 0, 0},
    Int3D{0, 1, 0},

    Int3D{0, -1, 1},
    Int3D{0, 0, 1},
    Int3D{0, 1, 1},
  };
  three_by_three_grid_vectors_[static_cast<int>(Axis::y)] = {
    Int3D{-1, 0, -1},
    Int3D{0, 0, -1},
    Int3D{1, 0, -1},

    Int3D{-1, 0, 0},
    Int3D{0, 0, 0},
    Int3D{1, 0, 0},

    Int3D{-1, 0, 1},
    Int3D{0, 0, 1},
    Int3D{1, 0, 1},
  };
  three_by_three_grid_vectors_[static_cast<int>(Axis::z)] = {
    Int3D{-1, -1, 0},
    Int3D{0, -1, 0},
    Int3D{1, -1, 0},

    Int3D{-1, 0, 0},
    Int3D{0, 0, 0},
    Int3D{1, 0, 0},

    Int3D{-1, 1, 0},
    Int3D{0, 1, 0},
    Int3D{1, 1, 0},
  };
}

void MeshGenerator::mesh_noncube(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel, float lighting) {
  float i = position[0], j = position[1], k = position[2];
  std::uint32_t seed = 0;
  seed ^= 0x9e3779b9 + (int)i;
  seed ^= 0x9e3779b9 + (int)j;
  seed ^= 0x9e3779b9 + (int)k;
  switch (voxel) {

  case Voxel::grass:
    float r = Common::RangeRand(-0.2f, 0.4f, seed);
    i += r;
    k += r;

    int layer = static_cast<int>(VoxelTexture::standing_grass);
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer, lighting});
    // cross
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, layer, lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer, lighting});
    break;
  }
}

std::array<float, 4> MeshGenerator::get_lighting(Region& region, Int3D coord, Axis axis) const {
  std::array<float, 4> lighting;
  std::array<float, 9> voxel_light_levels;
  auto& offsets = three_by_three_grid_vectors_[static_cast<int>(axis)];
  for (int i = 0; i < 9; ++i) {
    auto& offset = offsets[i];
    auto light_level = region.get_lighting(coord[0] + offset[0], coord[1] + offset[1], coord[2] + offset[2]);
    voxel_light_levels[i] = lighting_levels_[static_cast<int>(light_level)];
  }
  lighting[0] = (voxel_light_levels[0] + voxel_light_levels[1] + voxel_light_levels[3] + voxel_light_levels[4]) / 4;
  lighting[1] = (voxel_light_levels[1] + voxel_light_levels[2] + voxel_light_levels[4] + voxel_light_levels[5]) / 4;
  lighting[2] = (voxel_light_levels[3] + voxel_light_levels[4] + voxel_light_levels[6] + voxel_light_levels[7]) / 4;
  lighting[3] = (voxel_light_levels[4] + voxel_light_levels[5] + voxel_light_levels[7] + voxel_light_levels[8]) / 4;

  return lighting;
}

void MeshGenerator::mesh_chunk(Region& region, const Location& location) {
  auto& chunk = region.get_chunk(location);
  auto adjacent_chunks = region.get_adjacent_chunks(location);
  auto& mesh = meshes_[location];
  mesh.reserve(defacto_vertices_per_mesh);
  glm::vec3 chunk_position(
    (location[0] - origin_[0]) * Chunk::sz_x, (location[1] - origin_[1]) * Chunk::sz_y, (location[2] - origin_[2]) * Chunk::sz_z);
  Int3D global = Int3D{location[0] * Chunk::sz_x, location[1] * Chunk::sz_y, location[2] * Chunk::sz_z};
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int y = 0; y < Chunk::sz_y; ++y) {
      for (int x = 0; x < Chunk::sz_x; ++x) {
        auto voxel = chunk.get_voxel(x, y, z);
        if (voxel < Voxel::WATER_UPPER)
          continue;

        auto position = chunk_position + glm::vec3(x, y, z);
        if (voxel < Voxel::CUBE_LOWER) {
          auto level = chunk.get_lighting(x,y,z);
          auto lighting = lighting_levels_[level];
          mesh_noncube(mesh, position, voxel, lighting);
          continue;
        }

        std::array<Voxel, 6> adjacent;
        if (x > 0) {
          adjacent[nx] = chunk.get_voxel(x - 1, y, z);
        } else {
          adjacent[nx] = adjacent_chunks[nx]->get_voxel(Chunk::sz_x - 1, y, z);
        }
        if (x < Chunk::sz_x - 1) {
          adjacent[px] = chunk.get_voxel(x + 1, y, z);
        } else {
          adjacent[px] = adjacent_chunks[px]->get_voxel(0, y, z);
        }
        if (y > 0) {
          adjacent[ny] = chunk.get_voxel(x, y - 1, z);
        } else {
          adjacent[ny] = adjacent_chunks[ny]->get_voxel(x, Chunk::sz_y - 1, z);
        }
        if (y < Chunk::sz_y - 1) {
          adjacent[py] = chunk.get_voxel(x, y + 1, z);
        } else {
          adjacent[py] = adjacent_chunks[py]->get_voxel(x, 0, z);
        }
        if (z > 0) {
          adjacent[nz] = chunk.get_voxel(x, y, z - 1);
        } else {
          adjacent[nz] = adjacent_chunks[nz]->get_voxel(x, y, Chunk::sz_z - 1);
        }
        if (z < Chunk::sz_z - 1) {
          adjacent[pz] = chunk.get_voxel(x, y, z + 1);
        } else {
          adjacent[pz] = adjacent_chunks[pz]->get_voxel(x, y, 0);
        }

        std::array<VoxelTexture, 6> layers;
        switch (voxel) {
        case Voxel::dirt:
          if (adjacent[py] < Voxel::OPAQUE_LOWER) {
            layers[nx] = VoxelTexture::grass;
            layers[px] = VoxelTexture::grass;
            layers[nz] = VoxelTexture::grass;
            layers[pz] = VoxelTexture::grass;
          } else {
            layers[nx] = VoxelTexture::dirt;
            layers[px] = VoxelTexture::dirt;
            layers[nz] = VoxelTexture::dirt;
            layers[pz] = VoxelTexture::dirt;
          }
          if (adjacent[py] == Voxel::water_full) {
            layers[py] = VoxelTexture::grass;
          } else {
            layers[py] = VoxelTexture::grass;
          }
          layers[ny] = VoxelTexture::dirt;
          break;
        case Voxel::sand:
          layers.fill(VoxelTexture::sand);
          break;
        case Voxel::tree_trunk:
          layers.fill(VoxelTexture::tree_trunk);
          break;
        case Voxel::leaves:
          layers.fill(VoxelTexture::leaves);
          break;
        case Voxel::sandstone:
          layers.fill(VoxelTexture::sandstone);
          break;
        case Voxel::stone:
          layers.fill(VoxelTexture::stone);
          break;
        }

        auto TYPE_UPPER_BOUND = Voxel::num_voxel_types;
        if (voxel > Voxel::OPAQUE_LOWER)
          TYPE_UPPER_BOUND = Voxel::OPAQUE_LOWER;

        int i = position[0], j = position[1], k = position[2];
        auto& textures = reinterpret_cast<std::array<int, 6>&>(layers);

        if (adjacent[nx] < TYPE_UPPER_BOUND) {
          auto lighting = get_lighting(region, Int3D{global[0] + x - 1, global[1] + y, global[2] + z}, Axis::x);
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, textures[nx], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, textures[nx], lighting[3]});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tr, textures[nx], lighting[1]});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, textures[nx], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, textures[nx], lighting[2]});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, textures[nx], lighting[3]});
        }
        if (adjacent[px] < TYPE_UPPER_BOUND) {
          auto lighting = get_lighting(region, Int3D{global[0] + x + 1, global[1] + y, global[2] + z}, Axis::x);
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::bl, textures[px], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tl, textures[px], lighting[1]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, textures[px], lighting[3]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::bl, textures[px], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, textures[px], lighting[3]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, textures[px], lighting[2]});
        }
        if (adjacent[ny] < TYPE_UPPER_BOUND) {
          auto lighting = get_lighting(region, Int3D{global[0] + x, global[1] + y - 1, global[2] + z}, Axis::y);
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, textures[ny], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::tr, textures[ny], lighting[3]});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::tl, textures[ny], lighting[2]});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, textures[ny], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::tl, textures[ny], lighting[1]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::bl, textures[ny], lighting[3]});
        }
        if (adjacent[py] < TYPE_UPPER_BOUND) {
          auto lighting = get_lighting(region, Int3D{global[0] + x, global[1] + y + 1, global[2] + z}, Axis::y);
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::br, textures[py], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tl, textures[py], lighting[3]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, textures[py], lighting[1]});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::br, textures[py], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::bl, textures[py], lighting[2]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tl, textures[py], lighting[3]});
        }
        if (adjacent[nz] < TYPE_UPPER_BOUND) {
          auto lighting = get_lighting(region, Int3D{global[0] + x, global[1] + y, global[2] + z - 1}, Axis::z);
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, textures[nz], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, textures[nz], lighting[2]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, textures[nz], lighting[3]});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, textures[nz], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, textures[nz], lighting[3]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, textures[nz], lighting[1]});
        }
        if (adjacent[pz] < TYPE_UPPER_BOUND) {
          auto lighting = get_lighting(region, Int3D{global[0] + x, global[1] + y, global[2] + z + 1}, Axis::z);
          mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::br, textures[pz], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tl, textures[pz], lighting[3]});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tr, textures[pz], lighting[2]});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::br, textures[pz], lighting[0]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::bl, textures[pz], lighting[1]});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tl, textures[pz], lighting[3]});
        }
      }
    }
  }
}

void MeshGenerator::consume_region(Region& region) {
  auto& diffs = region.get_diffs();
  /*   int num_meshed = 0;
    float total_duration = 0.0; */

  for (auto& diff : diffs) {
    auto& loc = diff.location;

    if (!origin_set_) {
      origin_ = loc;
      origin_set_ = true;
      diffs_.emplace_back(Diff{origin_, Diff::origin});
    }

    if (diff.kind == Region::Diff::creation) {
      auto& chunk = region.get_chunk(loc);

      //      auto start = std::chrono::high_resolution_clock::now();
      random_seed_ = 0;
      mesh_chunk(region, loc);
      diffs_.emplace_back(Diff{loc, Diff::creation});
      
/*             auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "mesh time: " << duration.count() << std::endl */
      
      /*     num_meshed++;
          total_duration += duration.count(); */
    } else if (diff.kind == Region::Diff::deletion) {
      diffs_.emplace_back(Diff{loc, Diff::deletion});
    }
  }
  /* if (num_meshed > 0)
    std::cout << "Average execution time: " << total_duration / num_meshed << " microseconds" << std::endl; */
  region.clear_diffs();
}

const std::vector<MeshGenerator::Diff>& MeshGenerator::get_diffs() const {
  return diffs_;
}

void MeshGenerator::clear_diffs() {
  meshes_.clear();
  diffs_.clear();
}

const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& MeshGenerator::get_meshes() const {
  return meshes_;
}