#ifndef TYPES_H
#define TYPES_H

#include <any>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <glm/glm.hpp>

namespace {
  constexpr unsigned int create_bitmask(int start, int end) {
    unsigned int mask = 0;
    for (int i = start; i <= end; ++i) {
      mask |= (1 << i);
    }
    return mask;
  }
} // namespace

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

  friend std::ostream& operator<<(std::ostream& os, const Location& loc) {
    os << "(" + std::to_string(loc.coordinates[0]) + "," + std::to_string(loc.coordinates[1]) + "," + std::to_string(loc.coordinates[2]) + ")";
    return os;
  }

  Location& operator+=(const Location& other) {
    coordinates[0] += other.coordinates[0];
    coordinates[1] += other.coordinates[1];
    coordinates[2] += other.coordinates[2];
    return *this;
  }

  Location operator*(int i) const {
    Location result;
    result.coordinates[0] = coordinates[0] * i;
    result.coordinates[1] = coordinates[1] * i;
    result.coordinates[2] = coordinates[2] * i;
    return result;
  }

  Location operator+(const Location& other) {
    Location result;
    result[0] = coordinates[0] + other.coordinates[0];
    result[1] = coordinates[1] + other.coordinates[1];
    result[2] = coordinates[2] + other.coordinates[2];
    return result;
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

struct Location2DHash {
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

class NewVertex {
public:
  std::uint32_t data_;
  float lighting_;

  void set_position(int x, int y, int z) {
    data_ |= (x & xpos_mask);
    data_ |= (y & ypos_mask);
    data_ |= (z & zpos_mask);
  }

  void set_lighting(float lighting) {
    lighting_ = lighting;
  }

  void set_texture(int texture) {
    data_ |= (texture & texture_mask);
  }

  void set_uvs(int uvs) {
    data_ |= (uvs & uvs_mask);
  }

  void set_normal(Direction dir) {
    data_ |= (static_cast<int>(dir) & normal_mask);
  }

private:
  static constexpr unsigned int xpos_mask = create_bitmask(0, 4);
  static constexpr unsigned int ypos_mask = create_bitmask(5, 9);
  static constexpr unsigned int zpos_mask = create_bitmask(10, 14);
  static constexpr unsigned int normal_mask = create_bitmask(15,17);
  static constexpr unsigned int uvs_mask = create_bitmask(18,19);
  static constexpr unsigned int texture_mask = create_bitmask(20, 31);
};

namespace QuadCoord {
  constexpr glm::vec2 bl = glm::vec2(0.f, 0.f);
  constexpr glm::vec2 br = glm::vec2(1.f, 0.f);
  constexpr glm::vec2 tl = glm::vec2(0.f, 1.f);
  constexpr glm::vec2 tr = glm::vec2(1.f, 1.f);
} // namespace QuadCoord

using Message = std::vector<uint8_t>;

enum class Item {
  empty,
  dirt,
  stone,
  sandstone
};

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

  num_voxel_types
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

struct Action {
  struct NewActiveItemData {
    Item item;
  };
  enum Kind {
    new_active_item,
    create_group // blah blah
  };
  Kind kind;
  std::any data;

  template <typename T>
  Action(Kind k, const T& obj) : kind(k), data(obj) {}
};

#endif
