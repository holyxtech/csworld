#include "mesh_generator.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <glm/ext.hpp>
#include "mesh_utils.h"

MeshGenerator::MeshGenerator() {}

std::array<Voxel, 6> MeshGenerator::get_adjacent_voxels(
  const Chunk& chunk, std::array<const Chunk*, 6>& adjacent_chunks, int x, int y, int z) const {
  std::array<Voxel, 6> adjacent;
  if (x > 0)
    adjacent[nx] = chunk.get_voxel(x - 1, y, z);
  else
    adjacent[nx] = adjacent_chunks[nx]->get_voxel(Chunk::sz_x - 1, y, z);
  if (x < Chunk::sz_x - 1)
    adjacent[px] = chunk.get_voxel(x + 1, y, z);
  else
    adjacent[px] = adjacent_chunks[px]->get_voxel(0, y, z);
  if (y > 0)
    adjacent[ny] = chunk.get_voxel(x, y - 1, z);
  else
    adjacent[ny] = adjacent_chunks[ny]->get_voxel(x, Chunk::sz_y - 1, z);
  if (y < Chunk::sz_y - 1)
    adjacent[py] = chunk.get_voxel(x, y + 1, z);
  else
    adjacent[py] = adjacent_chunks[py]->get_voxel(x, 0, z);
  if (z > 0)
    adjacent[nz] = chunk.get_voxel(x, y, z - 1);
  else
    adjacent[nz] = adjacent_chunks[nz]->get_voxel(x, y, Chunk::sz_z - 1);
  if (z < Chunk::sz_z - 1)
    adjacent[pz] = chunk.get_voxel(x, y, z + 1);
  else
    adjacent[pz] = adjacent_chunks[pz]->get_voxel(x, y, 0);
  return adjacent;
}


void MeshGenerator::mesh_noncube(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel) {
  float i = position[0], j = position[1], k = position[2];
  std::uint32_t seed = 0;
  seed ^= 0x9e3779b9 + (int)i;
  seed ^= 0x9e3779b9 + (int)j;
  seed ^= 0x9e3779b9 + (int)k;

  int layer;
  float r = Common::RangeRand(-0.2f, 0.4f, seed);
  i += r;
  k += r;
  switch (voxel) {
  case Voxel::grass:
    layer = static_cast<int>(IrregularTexture::standing_grass);
    break;
  case Voxel::roses:
    layer = static_cast<int>(IrregularTexture::roses);
    break;
  case Voxel::sunflower:
    layer = static_cast<int>(IrregularTexture::sunflower);
    break;
  }
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), QuadCoord::tr, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), QuadCoord::tl, layer});
  // cross
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), QuadCoord::tr, layer});
  mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, layer});
  mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), QuadCoord::tl, layer});
}

void MeshGenerator::mesh_water(std::vector<Vertex>& mesh, glm::vec3& position, Voxel voxel, std::array<Voxel, 6>& adjacent) {
  // presume the voxel is water_full for now
  float i = position[0], j = position[1], k = position[2];
  float delta = 0.06;

  // if there's water above you, then you need to be a full cube (subject to culling)
  // if no water above you, short water block
  float height = 1;
  if (!vops::is_water(adjacent[py]))
    height -= delta;

  int textureId = static_cast<int>(CubeTexture::water);

  if (!vops::is_opaque(adjacent[nx]) && !vops::is_water(adjacent[nx])) {
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j + height, k + 1), QuadCoord::tl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j + height, k), QuadCoord::tr, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::bl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j + height, k + 1), QuadCoord::tl, textureId});
  }
  if (!vops::is_opaque(adjacent[px]) && !vops::is_water(adjacent[px])) {
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::bl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k), QuadCoord::tl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k + 1), QuadCoord::tr, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::bl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k + 1), QuadCoord::tr, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::br, textureId});
  }
  if (!vops::is_opaque(adjacent[ny]) && !vops::is_water(adjacent[ny])) {
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::tr, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::tl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::br, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::tl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::bl, textureId});
  }
  if (!vops::is_opaque(adjacent[py]) && !vops::is_water(adjacent[py])) {
    mesh.emplace_back(Vertex{glm::vec3(i, j + height, k), QuadCoord::br, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k + 1), QuadCoord::tl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k), QuadCoord::tr, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j + height, k), QuadCoord::br, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j + height, k + 1), QuadCoord::bl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k + 1), QuadCoord::tl, textureId});
  }
  if (!vops::is_opaque(adjacent[nz]) && !vops::is_water(adjacent[nz])) {
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j + height, k), QuadCoord::tl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k), QuadCoord::tr, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k), QuadCoord::bl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k), QuadCoord::tr, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), QuadCoord::br, textureId});
  }
  if (!vops::is_opaque(adjacent[pz]) && !vops::is_water(adjacent[pz])) {
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::br, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k + 1), QuadCoord::tl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j + height, k + 1), QuadCoord::tr, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), QuadCoord::br, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), QuadCoord::bl, textureId});
    mesh.emplace_back(Vertex{glm::vec3(i + 1, j + height, k + 1), QuadCoord::tl, textureId});
  }
}

void MeshGenerator::mesh_chunk(const Region& region, const Location& location) {
  auto& chunk = region.get_chunk(location);
  auto adjacent_chunks = region.get_adjacent_chunks(location);
  auto& mesh = meshes_[location];
  auto& irregular_mesh = irregular_meshes_[location];
  auto& water_mesh = water_meshes_[location];
  mesh.reserve(defacto_vertices_per_mesh);
  glm::vec3 chunk_position(
    (location[0] - origin_[0]) * Chunk::sz_x, (location[1] - origin_[1]) * Chunk::sz_y, (location[2] - origin_[2]) * Chunk::sz_z);
  Int3D global = Int3D{location[0] * Chunk::sz_x, location[1] * Chunk::sz_y, location[2] * Chunk::sz_z};
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int y = 0; y < Chunk::sz_y; ++y) {
      for (int x = 0; x < Chunk::sz_x; ++x) {
        auto voxel = chunk.get_voxel(x, y, z);
        if (voxel == Voxel::empty)
          continue;

        auto position = chunk_position + glm::vec3(x, y, z);
        auto adjacent = get_adjacent_voxels(chunk, adjacent_chunks, x, y, z);

        if (vops::is_water(voxel)) {
          mesh_water(water_mesh, position, voxel, adjacent);
          continue;
        }

        if (!vops::is_cube(voxel)) {
          mesh_noncube(irregular_mesh, position, voxel);
          continue;
        }

        auto voxel_textures = MeshUtils::get_textures(voxel, adjacent);

        auto occluding_voxel_type = Voxel::voxel_enum_size;
        if (vops::is_opaque(voxel))
          occluding_voxel_type = Voxel::OPAQUE_LOWER;

        auto& textures = reinterpret_cast<std::array<int, 6>&>(voxel_textures);

        if (adjacent[nx] < occluding_voxel_type) {
          mesh.emplace_back(CubeVertex(x, y, z, nx, br, textures[nx]));
          mesh.emplace_back(CubeVertex(x, y + 1, z + 1, nx, tl, textures[nx]));
          mesh.emplace_back(CubeVertex(x, y + 1, z, nx, tr, textures[nx]));
          mesh.emplace_back(CubeVertex(x, y, z, nx, br, textures[nx]));
          mesh.emplace_back(CubeVertex(x, y, z + 1, nx, bl, textures[nx]));
          mesh.emplace_back(CubeVertex(x, y + 1, z + 1, nx, tl, textures[nx]));
        }
        if (adjacent[px] < occluding_voxel_type) {
          mesh.emplace_back(CubeVertex(x + 1, y, z, px, bl, textures[px]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z, px, tl, textures[px]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z + 1, px, tr, textures[px]));
          mesh.emplace_back(CubeVertex(x + 1, y, z, px, bl, textures[px]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z + 1, px, tr, textures[px]));
          mesh.emplace_back(CubeVertex(x + 1, y, z + 1, px, br, textures[px]));
        }
        if (adjacent[ny] < occluding_voxel_type) {
          mesh.emplace_back(CubeVertex(x, y, z, ny, br, textures[ny]));
          mesh.emplace_back(CubeVertex(x + 1, y, z + 1, ny, tr, textures[ny]));
          mesh.emplace_back(CubeVertex(x, y, z + 1, ny, tl, textures[ny]));
          mesh.emplace_back(CubeVertex(x, y, z, ny, br, textures[ny]));
          mesh.emplace_back(CubeVertex(x + 1, y, z, ny, tl, textures[ny]));
          mesh.emplace_back(CubeVertex(x + 1, y, z + 1, ny, bl, textures[ny]));
        }
        if (adjacent[py] < occluding_voxel_type) {
          mesh.emplace_back(CubeVertex(x, y + 1, z, py, br, textures[py]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z + 1, py, tl, textures[py]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z, py, tr, textures[py]));
          mesh.emplace_back(CubeVertex(x, y + 1, z, py, br, textures[py]));
          mesh.emplace_back(CubeVertex(x, y + 1, z + 1, py, bl, textures[py]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z + 1, py, tl, textures[py]));
        }
        if (adjacent[nz] < occluding_voxel_type) {
          mesh.emplace_back(CubeVertex(x, y, z, nz, bl, textures[nz]));
          mesh.emplace_back(CubeVertex(x, y + 1, z, nz, tl, textures[nz]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z, nz, tr, textures[nz]));
          mesh.emplace_back(CubeVertex(x, y, z, nz, bl, textures[nz]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z, nz, tr, textures[nz]));
          mesh.emplace_back(CubeVertex(x + 1, y, z, nz, br, textures[nz]));
        }
        if (adjacent[pz] < occluding_voxel_type) {
          mesh.emplace_back(CubeVertex(x, y, z + 1, pz, br, textures[pz]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z + 1, pz, tl, textures[pz]));
          mesh.emplace_back(CubeVertex(x, y + 1, z + 1, pz, tr, textures[pz]));
          mesh.emplace_back(CubeVertex(x, y, z + 1, pz, br, textures[pz]));
          mesh.emplace_back(CubeVertex(x + 1, y, z + 1, pz, bl, textures[pz]));
          mesh.emplace_back(CubeVertex(x + 1, y + 1, z + 1, pz, tl, textures[pz]));
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
  irregular_meshes_.clear();
  water_meshes_.clear();
  diffs_.clear();
}

const std::unordered_map<Location, std::vector<CubeVertex>, LocationHash>& MeshGenerator::get_meshes() const {
  return meshes_;
}

const Location& MeshGenerator::get_origin() const {
  return origin_;
}

const std::vector<CubeVertex> MeshGenerator::get_mesh(const Location& loc) const {
  return meshes_.at(loc);
}
const std::vector<Vertex> MeshGenerator::get_irregular_mesh(const Location& loc) const {
  return irregular_meshes_.at(loc);
}

const std::vector<Vertex> MeshGenerator::get_water_mesh(const Location& loc) const {
  return water_meshes_.at(loc);
}