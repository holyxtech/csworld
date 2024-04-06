#include "mesh_generator.h"
#include <chrono>
#include <cmath>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

std::array<float, 6> MeshGenerator::get_lighting(Chunk& chunk, std::array<Chunk*, 6>& adjacent_chunks, int x, int y, int z) const {
  std::array<float, 6> lighting;
  if (x > 0) {
    lighting[Direction::nx] = std::pow(light_decay, Chunk::max_lighting - chunk.get_lighting(x - 1, y, z));
  } else {
    lighting[Direction::nx] = std::pow(light_decay, Chunk::max_lighting - adjacent_chunks[Direction::nx]->get_lighting(Chunk::sz_x - 1, y, z));
  }
  if (x < Chunk::sz_x - 1) {
    lighting[Direction::px] = std::pow(light_decay, Chunk::max_lighting - chunk.get_lighting(x + 1, y, z));
  } else {
    lighting[Direction::px] = std::pow(light_decay, Chunk::max_lighting - adjacent_chunks[Direction::px]->get_lighting(0, y, z));
  }
  if (y > 0) {
    lighting[Direction::ny] = std::pow(light_decay, Chunk::max_lighting - chunk.get_lighting(x, y - 1, z));
  } else {
    lighting[Direction::ny] = std::pow(light_decay, Chunk::max_lighting - adjacent_chunks[Direction::ny]->get_lighting(x, Chunk::sz_y - 1, z));
  }
  if (y < Chunk::sz_y - 1) {
    lighting[Direction::py] = std::pow(light_decay, Chunk::max_lighting - chunk.get_lighting(x, y + 1, z));
  } else {
    lighting[Direction::py] = std::pow(light_decay, Chunk::max_lighting - adjacent_chunks[Direction::py]->get_lighting(x, 0, z));
  }
  if (z > 0) {
    lighting[Direction::nz] = std::pow(light_decay, Chunk::max_lighting - chunk.get_lighting(x, y, z - 1));
  } else {
    lighting[Direction::nz] = std::pow(light_decay, Chunk::max_lighting - adjacent_chunks[Direction::nz]->get_lighting(x, y, Chunk::sz_z - 1));
  }
  if (z < Chunk::sz_z - 1) {
    lighting[Direction::pz] = std::pow(light_decay, Chunk::max_lighting - chunk.get_lighting(x, y, z + 1));
  } else {
    lighting[Direction::pz] = std::pow(light_decay, Chunk::max_lighting - adjacent_chunks[Direction::pz]->get_lighting(x, y, 0));
  }
  /*  std::cout<<":"<<std::endl;
   for (int i =0 ;i<6;++i)
   std::cout<<lighting[i]<<std::endl; */
  return lighting;
}

// to speed this up, instead of a region ref, pass in a ref to an std::array<Chunk&, 6> or Chunk* or whatever
std::array<Voxel, 6> MeshGenerator::get_adjacent_voxels(
  Chunk& chunk, std::array<Chunk*, 6>& adjacent_chunks, int x, int y, int z) const {
  auto& location = chunk.get_location();
  std::array<Voxel, 6> adjacent{Voxel::empty};
  if (x > 0) {
    adjacent[Direction::nx] = chunk.get_voxel(x - 1, y, z);
  } else {
    adjacent[Direction::nx] = adjacent_chunks[Direction::nx]->get_voxel(Chunk::sz_x - 1, y, z);
  }
  if (x < Chunk::sz_x - 1) {
    adjacent[Direction::px] = chunk.get_voxel(x + 1, y, z);
  } else {
    adjacent[Direction::px] = adjacent_chunks[Direction::px]->get_voxel(0, y, z);
  }
  if (y > 0) {
    adjacent[Direction::ny] = chunk.get_voxel(x, y - 1, z);
  } else {
    adjacent[Direction::ny] = adjacent_chunks[Direction::ny]->get_voxel(x, Chunk::sz_y - 1, z);
  }
  if (y < Chunk::sz_y - 1) {
    adjacent[Direction::py] = chunk.get_voxel(x, y + 1, z);
  } else {
    adjacent[Direction::py] = adjacent_chunks[Direction::py]->get_voxel(x, 0, z);
  }
  if (z > 0) {
    adjacent[Direction::nz] = chunk.get_voxel(x, y, z - 1);
  } else {
    adjacent[Direction::nz] = adjacent_chunks[Direction::nz]->get_voxel(x, y, Chunk::sz_z - 1);
  }
  if (z < Chunk::sz_z - 1) {
    adjacent[Direction::pz] = chunk.get_voxel(x, y, z + 1);
  } else {
    adjacent[Direction::pz] = adjacent_chunks[Direction::pz]->get_voxel(x, y, 0);
  }
  return adjacent;
}

void MeshGenerator::fill_sides(
  std::vector<Vertex>& mesh, glm::vec3& position, std::array<Voxel, 6>& adjacent,
  std::array<VoxelTexture, 6>& layers, Voxel TYPE_UPPER_BOUND, std::array<float, 6>& lighting) {
  auto [nx, px, ny, py, nz, pz] = adjacent;
  auto [nx_layer, px_layer, ny_layer, py_layer, nz_layer, pz_layer] = reinterpret_cast<std::array<int, 6>&>(layers);
  auto [nx_lighting, px_lighting, ny_lighting, py_lighting, nz_lighting, pz_lighting] = lighting;
  int i = position[0], j = position[1], k = position[2];

  if (nx < TYPE_UPPER_BOUND) {
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, nx_layer, nx_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, nx_layer, nx_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tr, nx_layer, nx_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, nx_layer, nx_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, nx_layer, nx_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, nx_layer, nx_lighting});
  }
  if (px < TYPE_UPPER_BOUND) {
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::bl, px_layer, px_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tl, px_layer, px_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, px_layer, px_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::bl, px_layer, px_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, px_layer, px_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, px_layer, px_lighting});
  }
  if (ny < TYPE_UPPER_BOUND) {
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, ny_layer, ny_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::tr, ny_layer, ny_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::tl, ny_layer, ny_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, ny_layer, ny_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::tl, ny_layer, ny_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::bl, ny_layer, ny_lighting});
  }
  if (py < TYPE_UPPER_BOUND) {
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::br, py_layer, py_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tl, py_layer, py_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, py_layer, py_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::br, py_layer, py_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::bl, py_layer, py_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tl, py_layer, py_lighting});
  }
  if (nz < TYPE_UPPER_BOUND) {
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, nz_layer, nz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, nz_layer, nz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, nz_layer, nz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, nz_layer, nz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, nz_layer, nz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, nz_layer, nz_lighting});
  }
  if (pz < TYPE_UPPER_BOUND) {
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::br, pz_layer, pz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tl, pz_layer, pz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tr, pz_layer, pz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::br, pz_layer, pz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::bl, pz_layer, pz_lighting});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tl, pz_layer, pz_lighting});
  }
}

void MeshGenerator::mesh_noncube(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel) {
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
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer, 1.f});
    // cross
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, layer, 1.f});
    mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer, 1.f});
    break;
  }
}

void MeshGenerator::mesh_chunk(Region& region, const Location& location) {
  auto& chunk = region.get_chunk(location);
  auto adjacent_chunks = region.get_adjacent_chunks(location);
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

        auto adjacent = get_adjacent_voxels(chunk, adjacent_chunks, x, y, z);
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

        auto lighting = get_lighting(chunk, adjacent_chunks, x, y, z);

        if (voxel < Voxel::OPAQUE_LOWER)
          fill_sides(mesh, position, adjacent, layers, Voxel::voxel_enum_size, lighting);
        else
          fill_sides(mesh, position, adjacent, layers, Voxel::OPAQUE_LOWER, lighting);
      }
    }
  }
}

void MeshGenerator::mesh_water(Region& region, const Location& location) {
  auto& chunk = region.get_chunk(location);
  auto adjacent_chunks = region.get_adjacent_chunks(location);
  auto& water_voxels = chunk.get_water_voxels();
  auto& water_mesh = water_meshes_[location];
  glm::vec3 chunk_position(
    (location[0] - origin_[0]) * Chunk::sz_x, (location[1] - origin_[1]) * Chunk::sz_y, (location[2] - origin_[2]) * Chunk::sz_z);
  for (auto& idx : water_voxels) {
    auto [x, y, z] = chunk.flat_index_to_3d(idx);
    auto position = chunk_position + glm::vec3(x, y, z);
    auto adjacent = get_adjacent_voxels(chunk, adjacent_chunks, x, y, z);

    std::array<VoxelTexture, 6> layers;
    layers.fill(VoxelTexture::water);
    auto lighting = get_lighting(chunk, adjacent_chunks, x, y, z);

    fill_sides(water_mesh, position, adjacent, layers, Voxel::WATER_LOWER, lighting);
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
      mesh_water(region, loc);
      diffs_.emplace_back(Diff{loc, Diff::creation});
      /*
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "mesh time: " << duration.count() << std::endl;
       */
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
  water_meshes_.clear();
  diffs_.clear();
}

const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& MeshGenerator::get_meshes() const {
  return meshes_;
}
const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& MeshGenerator::get_water_meshes() const {
  return water_meshes_;
}