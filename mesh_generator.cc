#include "mesh_generator.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

namespace QuadCoord {
  constexpr glm::vec2 bl = glm::vec2(0.f, 0.f);
  constexpr glm::vec2 br = glm::vec2(1.f, 0.f);
  constexpr glm::vec2 tl = glm::vec2(0.f, 1.f);
  constexpr glm::vec2 tr = glm::vec2(1.f, 1.f);
} // namespace QuadCoord

enum Direction {
  nx,
  px,
  ny,
  py,
  nz,
  pz
};

std::array<int, 6> MeshGenerator::tex_layers(Voxel::VoxelType voxel, std::array<Voxel::VoxelType, 6> adjacent) const {
  std::array<int, 6> layers;

  auto [nx, px, ny, py, nz, pz] = adjacent;
  switch (voxel) {
  case Voxel::dirt:

    if (py == Voxel::empty) {
      layers[Direction::nx] = Voxel::textures.at(Voxel::tex_grass_side);
      layers[Direction::px] = Voxel::textures.at(Voxel::tex_grass_side);
      layers[Direction::nz] = Voxel::textures.at(Voxel::tex_grass_side);
      layers[Direction::pz] = Voxel::textures.at(Voxel::tex_grass_side);
    } else {
      layers[Direction::nx] = Voxel::textures.at(Voxel::tex_dirt);
      layers[Direction::px] = Voxel::textures.at(Voxel::tex_dirt);
      layers[Direction::nz] = Voxel::textures.at(Voxel::tex_dirt);
      layers[Direction::pz] = Voxel::textures.at(Voxel::tex_dirt);
    }
    layers[Direction::ny] = Voxel::textures.at(Voxel::tex_dirt);
    layers[Direction::py] = Voxel::textures.at(Voxel::tex_grass);
    break;
  }

  return layers;
}

std::array<Voxel::VoxelType, 6> MeshGenerator::get_adjacent_voxels(const Chunk& chunk, int x, int y, int z) const {
  std::array<Voxel::VoxelType, 6> adjacent;
  std::fill(adjacent.begin(), adjacent.end(), Voxel::empty);

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
  if (z < Chunk::sz_y - 1) {
    adjacent[Direction::pz] = chunk.get_voxel(x, y, z + 1);
  }

  return adjacent;
}

void MeshGenerator::mesh_chunk(const Chunk& chunk) {
  auto& location = chunk.get_location();
  auto& mesh = meshes_[location];

  glm::vec3 chunk_position(location[0] * Chunk::sz_x, location[1] * Chunk::sz_y, location[2] * Chunk::sz_z);
  int initial_mesh_size = mesh.size();
  for (int x = 0; x < Chunk::sz_x; ++x) {
    for (int y = 0; y < Chunk::sz_y; ++y) {
      for (int z = 0; z < Chunk::sz_z; ++z) {

        auto voxel = chunk.get_voxel(x, y, z);

        if (voxel == Voxel::empty)
          continue;
        auto position = chunk_position + glm::vec3(x, y, z);
        auto i = position.x, j = position.y, k = position.z;
        auto zv = glm::vec3(0.f);
        auto adjacent = get_adjacent_voxels(chunk, x, y, z);
        auto [nx, px, ny, py, nz, pz] = adjacent;
        auto [nx_layer, px_layer, ny_layer, py_layer, nz_layer, pz_layer] = tex_layers(voxel, adjacent);

        if (nx == Voxel::empty) {
          mesh.insert(
            mesh.end(),
            {{glm::vec3(i, j, k), zv, QuadCoord::br, nx_layer},
             {glm::vec3(i, j + 1, k), zv, QuadCoord::tr, nx_layer},
             {glm::vec3(i, j + 1, k + 1), zv, QuadCoord::tl, nx_layer},
             {glm::vec3(i, j, k), zv, QuadCoord::br, nx_layer},
             {glm::vec3(i, j + 1, k + 1), zv, QuadCoord::tl, nx_layer},
             {glm::vec3(i, j, k + 1), zv, QuadCoord::bl, nx_layer}});
        }
        if (px == Voxel::empty) {
          mesh.insert(
            mesh.end(),
            {{glm::vec3(i + 1, j, k), zv, QuadCoord::bl, px_layer},
             {glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tr, px_layer},
             {glm::vec3(i + 1, j + 1, k), zv, QuadCoord::tl, px_layer},
             {glm::vec3(i + 1, j, k), zv, QuadCoord::bl, px_layer},
             {glm::vec3(i + 1, j, k + 1), zv, QuadCoord::br, px_layer},
             {glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tr, px_layer}});
        }
        if (ny == Voxel::empty) {
          mesh.insert(
            mesh.end(),
            {{glm::vec3(i, j, k), zv, QuadCoord::tr, ny_layer},
             {glm::vec3(i, j, k + 1), zv, QuadCoord::br, ny_layer},
             {glm::vec3(i + 1, j, k + 1), zv, QuadCoord::bl, ny_layer},
             {glm::vec3(i, j, k), zv, QuadCoord::tr, ny_layer},
             {glm::vec3(i + 1, j, k + 1), zv, QuadCoord::bl, ny_layer},
             {glm::vec3(i + 1, j, k), zv, QuadCoord::tl, ny_layer}});
        }
        if (py == Voxel::empty) {
          mesh.insert(
            mesh.end(),
            {{glm::vec3(i, j + 1, k), zv, QuadCoord::bl, py_layer},
             {glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tr, py_layer},
             {glm::vec3(i, j + 1, k + 1), zv, QuadCoord::tl, py_layer},
             {glm::vec3(i, j + 1, k), zv, QuadCoord::bl, py_layer},
             {glm::vec3(i + 1, j + 1, k), zv, QuadCoord::br, py_layer},
             {glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tr, py_layer}});
        }
        if (nz == Voxel::empty) {
          mesh.insert(
            mesh.end(),
            {{glm::vec3(i, j, k), zv, QuadCoord::bl, nz_layer},
             {glm::vec3(i + 1, j + 1, k), zv, QuadCoord::tr, nz_layer},
             {glm::vec3(i, j + 1, k), zv, QuadCoord::tl, nz_layer},
             {glm::vec3(i, j, k), zv, QuadCoord::bl, nz_layer},
             {glm::vec3(i + 1, j, k), zv, QuadCoord::br, nz_layer},
             {glm::vec3(i + 1, j + 1, k), zv, QuadCoord::tr, nz_layer}});
        }
        if (pz == Voxel::empty) {
          mesh.insert(
            mesh.end(),
            {{glm::vec3(i, j, k + 1), zv, QuadCoord::br, pz_layer},
             {glm::vec3(i, j + 1, k + 1), zv, QuadCoord::tr, pz_layer},
             {glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tl, pz_layer},
             {glm::vec3(i, j, k + 1), zv, QuadCoord::br, pz_layer},
             {glm::vec3(i + 1, j + 1, k + 1), zv, QuadCoord::tl, pz_layer},
             {glm::vec3(i + 1, j, k + 1), zv, QuadCoord::bl, pz_layer}});
        }
      }
    }
  }
  diffs_.emplace_back(Diff{location, Diff::creation});
}

void MeshGenerator::consume_region(Region& region) {
  auto& diffs = region.get_diffs();
  for (auto& diff : diffs) {
    auto& loc = diff.loc;

    if (diff.kind == Region::Diff::creation) {
      auto& chunk = *region.get_chunk(loc);
      mesh_chunk(chunk);

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
  // process all the deletions...
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