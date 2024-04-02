#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <glm/glm.hpp>

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

using Int3D = Location;

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
    std::hash<int> hasher;
    std::size_t hashValue = 0;
    hashValue ^= hasher(l[0]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    hashValue ^= hasher(l[1]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    hashValue ^= hasher(l[2]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    return hashValue;
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
  glm::vec2 uvs;
  int layer;
  float lighting;
};

using Message = std::vector<uint8_t>;

enum class Voxel {
  empty,
  WATER_LOWER,
  water_full,
  WATER_UPPER,

  grass,

  CUBE_LOWER,
  glass,

  PARTIAL_OPAQUE_LOWER,
  leaves,

  OPAQUE_LOWER,
  dirt,
  sand,
  tree_trunk,
  sandstone,
  stone,

  voxel_enum_size
};

enum class VoxelTexture {
  dirt,
  grass,
  grass_side,
  water,
  sand,
  tree_trunk,
  leaves,
  sandstone,
  stone,
  standing_grass,

  num_voxel_textures
};

enum Direction {
  nx,
  px,
  ny,
  py,
  nz,
  pz
};

#endif
