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

std::array<int, 6> MeshGenerator::tex_layers(Voxel::VoxelType voxel, const std::array<Voxel::VoxelType, 6>& adjacent) const {
  std::array<int, 6> layers;
  // auto [nx, px, ny, py, nz, pz] = adjacent;
  switch (voxel) {
  case Voxel::dirt:

    if (adjacent[Direction::py] == Voxel::empty) {
      layers[Direction::nx] = Voxel::tex_grass_side;
      layers[Direction::px] = Voxel::tex_grass_side;
      layers[Direction::nz] = Voxel::tex_grass_side;
      layers[Direction::pz] = Voxel::tex_grass_side;
    } else {
      layers[Direction::nx] = Voxel::tex_dirt;
      layers[Direction::px] = Voxel::tex_dirt;
      layers[Direction::nz] = Voxel::tex_dirt;
      layers[Direction::pz] = Voxel::tex_dirt;
    }
    layers[Direction::ny] = Voxel::tex_dirt;
    layers[Direction::py] = Voxel::tex_grass;
    break;
  }

  return layers;
}

std::array<Voxel::VoxelType, 6> MeshGenerator::get_adjacent_voxels(const Chunk& chunk, int x, int y, int z) const {
  std::array<Voxel::VoxelType, 6> adjacent{Voxel::empty};

  if (x > 0) {
    adjacent[Direction::nx] = chunk.get_voxel(x - 1, y, z);
  }
  if (x < Chunk::sz_x - 1) {
    adjacent[Direction::px] = chunk.get_voxel(x + 1, y, z);
  }
  if (y > 0) {
    adjacent[Direction::ny] = chunk.get_voxel(x, y - 1, z);
  }
  if (y < Chunk::sz_y - 1) {
    adjacent[Direction::py] = chunk.get_voxel(x, y + 1, z);
  }
  if (z > 0) {
    adjacent[Direction::nz] = chunk.get_voxel(x, y, z - 1);
  }
  if (z < Chunk::sz_z - 1) {
    adjacent[Direction::pz] = chunk.get_voxel(x, y, z + 1);
  }
  return adjacent;
}

void MeshGenerator::mesh_greedy(const Chunk& chunk) {
  // positive-x of chunk as one face, negative x of chunk as one face, increment x, ...
  // and then for y, and then for z
}

void MeshGenerator::mesh_chunk(const Chunk& chunk) {
  auto& location = chunk.get_location();
  auto& mesh = meshes_[location];
  mesh.reserve(Chunk::sz * 9);
  glm::vec3 chunk_position(location[0] * Chunk::sz_x, location[1] * Chunk::sz_y, location[2] * Chunk::sz_z);
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int y = 0; y < Chunk::sz_y; ++y) {
      for (int x = 0; x < Chunk::sz_x; ++x) {
        auto voxel = chunk.get_voxel(x, y, z);
        if (voxel == Voxel::empty)
          continue;
        auto position = chunk_position + glm::vec3(x, y, z);
        auto i = position.x, j = position.y, k = position.z;
        auto zv = glm::vec3(0.f);

        Voxel::VoxelType nx, px, ny, py, nz, pz;
        nx = px = ny = py = nz = pz = Voxel::empty;
        if (x > 0) {
          nx = chunk.get_voxel(x - 1, y, z);
        }
        if (x < Chunk::sz_x - 1) {
          px = chunk.get_voxel(x + 1, y, z);
        }
        if (y > 0) {
          ny = chunk.get_voxel(x, y - 1, z);
        }
        if (y < Chunk::sz_y - 1) {
          py = chunk.get_voxel(x, y + 1, z);
        }
        if (z > 0) {
          nz = chunk.get_voxel(x, y, z - 1);
        }
        if (z < Chunk::sz_z - 1) {
          pz = chunk.get_voxel(x, y, z + 1);
        }

        int nx_layer, px_layer, ny_layer, py_layer, nz_layer, pz_layer;
        switch (voxel) {
        case Voxel::dirt:
          if (py == Voxel::empty) {
            nx_layer = Voxel::tex_grass_side;
            px_layer = Voxel::tex_grass_side;
            nz_layer = Voxel::tex_grass_side;
            pz_layer = Voxel::tex_grass_side;
          } else {
            nx_layer = Voxel::tex_dirt;
            px_layer = Voxel::tex_dirt;
            nz_layer = Voxel::tex_dirt;
            pz_layer = Voxel::tex_dirt;
          }
          ny_layer = Voxel::tex_dirt;
          py_layer = Voxel::tex_grass;
          break;
        }

        if (nx == Voxel::empty) {
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), zv, QuadCoord::br, nx_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), zv, QuadCoord::tr, nx_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), zv, QuadCoord::tl, nx_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), zv, QuadCoord::br, nx_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), zv, QuadCoord::tl, nx_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), zv, QuadCoord::bl, nx_layer});
        }
        if (px == Voxel::empty) {
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), zv, QuadCoord::bl, px_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tr, px_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), zv, QuadCoord::tl, px_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), zv, QuadCoord::bl, px_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), zv, QuadCoord::br, px_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tr, px_layer});
        }
        if (ny == Voxel::empty) {
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), zv, QuadCoord::tr, ny_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), zv, QuadCoord::br, ny_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), zv, QuadCoord::bl, ny_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), zv, QuadCoord::tr, ny_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), zv, QuadCoord::bl, ny_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), zv, QuadCoord::tl, ny_layer});
        }
        if (py == Voxel::empty) {
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), zv, QuadCoord::bl, py_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tr, py_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), zv, QuadCoord::tl, py_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), zv, QuadCoord::bl, py_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), zv, QuadCoord::br, py_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tr, py_layer});
        }
        if (nz == Voxel::empty) {
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), zv, QuadCoord::bl, nz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), zv, QuadCoord::tr, nz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k), zv, QuadCoord::tl, nz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k), zv, QuadCoord::bl, nz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k), zv, QuadCoord::br, nz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k), zv, QuadCoord::tr, nz_layer});
        }
        if (pz == Voxel::empty) {
          mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), zv, QuadCoord::br, pz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j + 1, k + 1), zv, QuadCoord::tr, pz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tl, pz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i, j, k + 1), zv, QuadCoord::br, pz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tl, pz_layer});
          mesh.emplace_back(Vertex{glm::vec3(i + 1, j, k + 1), zv, QuadCoord::bl, pz_layer});
        }
      }
    }
  }
  diffs_.emplace_back(Diff{location, Diff::creation});
}

void MeshGenerator::consume_region(Region& region) {
  auto& diffs = region.get_diffs();
  for (auto& diff : diffs) {
    auto& loc = diff.location;

    if (diff.kind == Region::Diff::creation) {
      auto& chunk = region.get_chunk(loc);
      //      auto start = std::chrono::high_resolution_clock::now();
      mesh_chunk(chunk);
      /*       auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl; */
    } else if (diff.kind == Region::Diff::deletion) {
      diffs_.emplace_back(Diff{loc, Diff::deletion});
    }
  }
  region.clear_diffs();
}

const std::vector<MeshGenerator::Diff>& MeshGenerator::get_diffs() const {
  return diffs_;
}

void MeshGenerator::clear_diffs() {
  for (auto& diff : diffs_) {
    auto& loc = diff.location;
    if (diff.kind == MeshGenerator::Diff::deletion) {
      meshes_.erase(loc);
    }
  }
  diffs_.clear();
}

const std::unordered_map<Location, std::vector<Vertex>, LocationHash>& MeshGenerator::get_meshes() const {
  return meshes_;
}