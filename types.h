#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

// using Location = std::array<int, 3>;
using Location2D = std::array<int, 2>;

class Location {
public:
  std::array<int, 3> coordinates;

  Location() {}

  Location(std::initializer_list<int> values) {
    auto it = values.begin();
    coordinates[0] = *it++;
    coordinates[1] = *it++;
    coordinates[2] = *it;
  }

  int& operator[](std::size_t index) {
    return coordinates[index];
  }

  const int& operator[](std::size_t index) const {
    return coordinates[index];
  }

  bool operator==(const Location& other) const {
    return coordinates == other.coordinates;
  }

  std::string repr() const {
    return "(" + std::to_string(coordinates[0]) + "," + std::to_string(coordinates[1]) + "," + std::to_string(coordinates[2]) + ")";
  }

  void print() const {
    std::cout << "(" << coordinates[0] << "," << coordinates[1] << "," << coordinates[2] << ")" << std::endl;
  }
};

struct LocationMath {
  static double distance(Location l1, Location l2) {
    int x_squared = (l1[0] - l2[0]) * (l1[0] - l2[0]);
    int y_squared = (l1[1] - l2[1]) * (l1[1] - l2[1]);
    int z_squared = (l1[2] - l2[2]) * (l1[2] - l2[2]);
    return std::sqrt(x_squared + y_squared + z_squared);
  }
  static int difference(Location l1, Location l2) {
    return std::abs(l1[0] - l2[0]) + std::abs(l1[1] - l2[1]) + std::abs(l1[2] - l2[2]);
  }
};

struct LocationHash {
  std::size_t operator()(const Location& l) const {
    std::hash<int> h;
    int ret = 3;
    ret ^= h(l[0]) | l[0];
    ret ^= h(l[1]) | l[1];
    ret ^= h(l[2]) | l[2];
    return ret;
  }
};

struct Location2DHash final {
  template <class T, std::size_t N>
  size_t operator()(const std::array<T, N>& arr) const noexcept {
    uintmax_t hash = std::hash<T>{}(arr[0]);
    hash <<= sizeof(uintmax_t) * 4;
    hash ^= std::hash<T>{}(arr[1]);
    return std::hash<uintmax_t>{}(hash);
  }
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uvs;
  int layer;
};

using Message = std::vector<uint8_t>;

namespace Voxel {
  enum VoxelType {
    empty,

    WATER_LOWER,
    water_full,
    WATER_UPPER,

    leaves,

    OPAQUE_LOWER,
    dirt,
    sand,
    tree_trunk,

    num_voxel_types
  };

  enum VoxelTexture {
    tex_dirt,
    tex_grass,
    tex_grass_side,
    tex_water,
    tex_sand,
    tex_tree_trunk,
    tex_leaves,

    num_voxel_textures
  };

} // namespace Voxel

enum Direction {
  nx,
  px,
  ny,
  py,
  nz,
  pz
};

/* struct Section {
  Location2D location;
  int elevation;
}; */

#endif
