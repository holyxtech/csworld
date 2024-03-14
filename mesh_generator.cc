#include "mesh_generator.h"
#include <chrono>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

namespace QuadCoord {
  constexpr glm::vec2 bl = glm::vec2(0.f, 0.f);
  constexpr glm::vec2 br = glm::vec2(1.f, 0.f);
  constexpr glm::vec2 tl = glm::vec2(0.f, 1.f);
  constexpr glm::vec2 tr = glm::vec2(1.f, 1.f);
} // namespace QuadCoord

std::array<Voxel, 6> MeshGenerator::get_adjacent_voxels(
  Region& region, Chunk& chunk, int x, int y, int z) const {
  auto& location = chunk.get_location();
  std::array<Voxel, 6> adjacent{Voxel::empty};
  if (x > 0) {
    adjacent[Direction::nx] = chunk.get_voxel(x - 1, y, z);
  } else {

    adjacent[Direction::nx] = region.get_voxel(
      location[0] * Chunk::sz_x - 1, location[1] * Chunk::sz_y + y, location[2] * Chunk::sz_z + z);
  }
  if (x < Chunk::sz_x - 1) {
    adjacent[Direction::px] = chunk.get_voxel(x + 1, y, z);
  } else {

    adjacent[Direction::px] = region.get_voxel(
      (location[0] + 1) * Chunk::sz_x, location[1] * Chunk::sz_y + y, location[2] * Chunk::sz_z + z);
  }
  if (y > 0) {
    adjacent[Direction::ny] = chunk.get_voxel(x, y - 1, z);
  } else {

    adjacent[Direction::ny] = region.get_voxel(
      location[0] * Chunk::sz_x + x, location[1] * Chunk::sz_y - 1, location[2] * Chunk::sz_z + z);
  }
  if (y < Chunk::sz_y - 1) {
    adjacent[Direction::py] = chunk.get_voxel(x, y + 1, z);
  } else {
    adjacent[Direction::py] = region.get_voxel(
      location[0] * Chunk::sz_x + x, (location[1] + 1) * Chunk::sz_y, location[2] * Chunk::sz_z + z);
  }
  if (z > 0) {
    adjacent[Direction::nz] = chunk.get_voxel(x, y, z - 1);
  } else {

    adjacent[Direction::nz] = region.get_voxel(
      location[0] * Chunk::sz_x + x, location[1] * Chunk::sz_y + y, location[2] * Chunk::sz_z - 1);
  }
  if (z < Chunk::sz_z - 1) {
    adjacent[Direction::pz] = chunk.get_voxel(x, y, z + 1);
  } else {

    adjacent[Direction::pz] = region.get_voxel(
      location[0] * Chunk::sz_x + x, location[1] * Chunk::sz_y + y, (location[2] + 1) * Chunk::sz_z);
  }
  return adjacent;
}

void MeshGenerator::mesh_greedy(const Chunk& chunk) {
  // positive-x of chunk as one face, negative x of chunk as one face, increment x, ...
  // and then for y, and then for z
}

void MeshGenerator::fill_sides(std::vector<Vertex>& mesh, glm::vec3& position, std::array<Voxel, 6>& adjacent, std::array<VoxelTexture, 6>& layers, Voxel TYPE_UPPER_BOUND) {
  auto [nx, px, ny, py, nz, pz] = adjacent;
  auto [nx_layer, px_layer, ny_layer, py_layer, nz_layer, pz_layer] = reinterpret_cast<std::array<int, 6>&>(layers);
  int i = position[0], j = position[1], k = position[2];

  if (nx < TYPE_UPPER_BOUND) {
    auto normal = glm::vec3(-1.f, 0.f, 0.f);
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), normal, QuadCoord::br, nx_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), normal, QuadCoord::tl, nx_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), normal, QuadCoord::tr, nx_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), normal, QuadCoord::br, nx_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), normal, QuadCoord::bl, nx_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), normal, QuadCoord::tl, nx_layer});
  }
  if (px < TYPE_UPPER_BOUND) {
    auto normal = glm::vec3(1.f, 0.f, 0.f);
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), normal, QuadCoord::bl, px_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), normal, QuadCoord::tl, px_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), normal, QuadCoord::tr, px_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), normal, QuadCoord::bl, px_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), normal, QuadCoord::tr, px_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), normal, QuadCoord::br, px_layer});
  }
  if (ny < TYPE_UPPER_BOUND) {
    auto normal = glm::vec3(0.f, -1.f, 0.f);
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), normal, QuadCoord::br, ny_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), normal, QuadCoord::tr, ny_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), normal, QuadCoord::tl, ny_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), normal, QuadCoord::br, ny_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), normal, QuadCoord::tl, ny_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), normal, QuadCoord::bl, ny_layer});
  }
  if (py < TYPE_UPPER_BOUND) {
    auto normal = glm::vec3(0.f, 1.f, 0.f);
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), normal, QuadCoord::br, py_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), normal, QuadCoord::tl, py_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), normal, QuadCoord::tr, py_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), normal, QuadCoord::br, py_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), normal, QuadCoord::bl, py_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), normal, QuadCoord::tl, py_layer});
  }
  if (nz < TYPE_UPPER_BOUND) {
    auto normal = glm::vec3(0.f, 0.f, -1.f);
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), normal, QuadCoord::bl, nz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), normal, QuadCoord::tl, nz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), normal, QuadCoord::tr, nz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), normal, QuadCoord::bl, nz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), normal, QuadCoord::tr, nz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), normal, QuadCoord::br, nz_layer});
  }
  if (pz < TYPE_UPPER_BOUND) {
    auto normal = glm::vec3(0.f, 0.f, 1.f);
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), normal, QuadCoord::br, pz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), normal, QuadCoord::tl, pz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), normal, QuadCoord::tr, pz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), normal, QuadCoord::br, pz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), normal, QuadCoord::bl, pz_layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), normal, QuadCoord::tl, pz_layer});
  }
}

void MeshGenerator::mesh_noncube(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel) {
  float i = position[0], j = position[1], k = position[2];
  switch (voxel) {
  case Voxel::grass:
    // emplace back the triangles
    float r = Common::random_float(-0.2f, 0.2f);
    i += r;
    k += r;

    auto normal = glm::vec3(0.f);
    int layer = static_cast<int>(VoxelTexture::standing_grass);
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), normal, QuadCoord::br, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), normal, QuadCoord::tr, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), normal, QuadCoord::tl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), normal, QuadCoord::br, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), normal, QuadCoord::tl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), normal, QuadCoord::bl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), normal, QuadCoord::br, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), normal, QuadCoord::tl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), normal, QuadCoord::tr, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), normal, QuadCoord::br, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), normal, QuadCoord::bl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), normal, QuadCoord::tl, layer});

    // cross
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), normal, QuadCoord::br, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), normal, QuadCoord::tr, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), normal, QuadCoord::tl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), normal, QuadCoord::br, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), normal, QuadCoord::tl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), normal, QuadCoord::bl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), normal, QuadCoord::br, layer});

    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), normal, QuadCoord::tl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), normal, QuadCoord::tr, layer});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), normal, QuadCoord::br, layer});

    mesh.emplace_back(Vertex{glm::vec3(i, j, k), normal, QuadCoord::bl, layer});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), normal, QuadCoord::tl, layer});
    break;
  }
}

void MeshGenerator::mesh_chunk(Region& region, const Location& location) {
  auto& chunk = region.get_chunk(location);
  auto& mesh = meshes_[location];
  mesh.reserve(default_max_vertices);
  glm::vec3 chunk_position(
    (location[0] - origin_[0]) * Chunk::sz_x, (location[1] - origin_[1]) * Chunk::sz_y, (location[2] - origin_[2]) * Chunk::sz_z);
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int y = 0; y < Chunk::sz_y; ++y) {
      for (int x = 0; x < Chunk::sz_x; ++x) {
        auto voxel = chunk.get_voxel(x, y, z);

        if (voxel < Voxel::WATER_UPPER) {
          continue;
        }
        auto position = chunk_position + glm::vec3(x, y, z);
        if (voxel < Voxel::CUBE_LOWER) {
          mesh_noncube(mesh, position, voxel);
          continue;
        }

        auto adjacent = get_adjacent_voxels(region, chunk, x, y, z);
        std::array<VoxelTexture, 6> layers;
        switch (voxel) {
        case Voxel::dirt:
          if (adjacent[Direction::py] < Voxel::OPAQUE_LOWER) {
            layers[Direction::nx] = VoxelTexture::grass_side;
            layers[Direction::px] = VoxelTexture::grass_side;
            layers[Direction::nz] = VoxelTexture::grass_side;
            layers[Direction::pz] = VoxelTexture::grass_side;
          } else {
            layers[Direction::nx] = VoxelTexture::dirt;
            layers[Direction::px] = VoxelTexture::dirt;
            layers[Direction::nz] = VoxelTexture::dirt;
            layers[Direction::pz] = VoxelTexture::dirt;
          }
          if (adjacent[Direction::py] == Voxel::water_full) {
            layers[Direction::py] = VoxelTexture::grass;
          } else {
            layers[Direction::py] = VoxelTexture::grass;
          }
          layers[Direction::ny] = VoxelTexture::dirt;
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
        if (voxel < Voxel::OPAQUE_LOWER)
          fill_sides(mesh, position, adjacent, layers, Voxel::num_voxel_types);
        else
          fill_sides(mesh, position, adjacent, layers, Voxel::OPAQUE_LOWER);
      }
    }
  }
  diffs_.emplace_back(Diff{location, Diff::creation});
}

void MeshGenerator::mesh_water(Region& region, const Location& location) {
  auto& chunk = region.get_chunk(location);
  auto& water_voxels = chunk.get_water_voxels();
  auto& water_mesh = water_meshes_[location];
  glm::vec3 chunk_position(
    (location[0] - origin_[0]) * Chunk::sz_x, (location[1] - origin_[1]) * Chunk::sz_y, (location[2] - origin_[2]) * Chunk::sz_z);
  for (auto& idx : water_voxels) {
    auto [x, y, z] = chunk.flat_index_to_3d(idx);
    auto position = chunk_position + glm::vec3(x, y, z);
    auto adjacent = get_adjacent_voxels(region, chunk, x, y, z);

    std::array<VoxelTexture, 6> layers;
    layers.fill(VoxelTexture::water);
    fill_sides(water_mesh, position, adjacent, layers, Voxel::WATER_LOWER);
  }
  diffs_.emplace_back(Diff{location, Diff::water});
}

void MeshGenerator::consume_region(Region& region) {
  auto& diffs = region.get_diffs();
  /* int num_meshed = 0;
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

      // auto start = std::chrono::high_resolution_clock::now();
      mesh_chunk(region, loc);
      /*       auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); */
      // std::cout << "mesh time: " << duration.count() << std::endl;

      /* num_meshed++;
      total_duration += duration.count(); */
    } else if (diff.kind == Region::Diff::deletion) {
      diffs_.emplace_back(Diff{loc, Diff::deletion});
    } else if (diff.kind == Region::Diff::water) {
      auto& chunk = region.get_chunk(loc);
      mesh_water(region, loc);
    }
  }
  // std::cout << "Average execution time: " << total_duration / num_meshed << " microseconds" << std::endl;
  region.clear_diffs();
}

const std::vector<MeshGenerator::Diff>& MeshGenerator::get_diffs() const {
  return diffs_;
}

void MeshGenerator::clear_diffs() {
  meshes_.clear();
  water_meshes_.clear();
  diffs_.clear();
}

const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& MeshGenerator::get_meshes() const {
  return meshes_;
}
const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& MeshGenerator::get_water_meshes() const {
  return water_meshes_;
}