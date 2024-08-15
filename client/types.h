#ifndef TYPES_H
#define TYPES_H

#include <optional>
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
#include "item.h"
#include "common.h"

enum Direction {
  nx,
  px,
  ny,
  py,
  nz,
  pz
};

enum QuadCorner {
  bl,
  br,
  tl,
  tr
};

using Location2D = std::array<int, 2>;
using Int2D = Location2D;

class Location {
public:
  std::array<int, 3> coordinates;

  // Default constructor
  constexpr Location() : coordinates{0, 0, 0} {}

  // Constructor with initializer list
  constexpr Location(std::initializer_list<int> values) {
    auto it = values.begin();
    coordinates[0] = (it != values.end()) ? *it++ : 0;
    coordinates[1] = (it != values.end()) ? *it++ : 0;
    coordinates[2] = (it != values.end()) ? *it : 0;
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

namespace LocationMath {
  static double distance(Location l1, Location l2) {
    int x_squared = (l1[0] - l2[0]) * (l1[0] - l2[0]);
    int y_squared = (l1[1] - l2[1]) * (l1[1] - l2[1]);
    int z_squared = (l1[2] - l2[2]) * (l1[2] - l2[2]);
    return std::sqrt(x_squared + y_squared + z_squared);
  }
  static double distance(Location2D l1, Location2D l2) {
    int x_squared = (l1[0] - l2[0]) * (l1[0] - l2[0]);
    int z_squared = (l1[1] - l2[1]) * (l1[1] - l2[1]);
    return std::sqrt(x_squared + z_squared);
  }
  static int difference(Location l1, Location l2) {
    return std::abs(l1[0] - l2[0]) + std::abs(l1[1] - l2[1]) + std::abs(l1[2] - l2[2]);
  }
  static std::array<Location, 6> get_adjacent_locations(const Location& loc) {
    return std::array<Location, 6>{
      Location{loc[0] - 1, loc[1], loc[2]},
      Location{loc[0] + 1, loc[1], loc[2]},
      Location{loc[0], loc[1] - 1, loc[2]},
      Location{loc[0], loc[1] + 1, loc[2]},
      Location{loc[0], loc[1], loc[2] - 1},
      Location{loc[0], loc[1], loc[2] + 1},
    };
  }
}; // namespace LocationMath

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
  glm::vec3 normal;
  glm::vec2 uvs;
  int textureId;
};

class LodVertex {
public:
  unsigned int data = 0;
  LodVertex(int x, int y, int z, Direction normal, QuadCorner uvs, int textureId) {
    data |= (x & xpos_mask);
    data |= ((y << 4) & ypos_mask);
    data |= ((z << 9) & zpos_mask);
    data |= ((normal << 13) & normal_mask);
    data |= ((uvs << 16) & uvs_mask);
    data |= ((textureId << 18) & texture_mask);
  }

private:
  static constexpr unsigned int xpos_mask = Common::create_bitmask(0, 3);
  static constexpr unsigned int ypos_mask = Common::create_bitmask(4, 8);
  static constexpr unsigned int zpos_mask = Common::create_bitmask(9, 12);
  static constexpr unsigned int normal_mask = Common::create_bitmask(13, 15);
  static constexpr unsigned int uvs_mask = Common::create_bitmask(16, 17);
  static constexpr unsigned int texture_mask = Common::create_bitmask(18, 31);
};

class CubeVertex {
public:
  unsigned int data = 0;

  CubeVertex(int x, int y, int z, Direction normal, QuadCorner uvs, int textureId) {
    data |= (x & xpos_mask);
    data |= ((y << 6) & ypos_mask);
    data |= ((z << 12) & zpos_mask);
    data |= ((normal << 18) & normal_mask);
    data |= ((uvs << 21) & uvs_mask);
    data |= ((textureId << 23) & texture_mask);
  }

private:
  static constexpr unsigned int xpos_mask = Common::create_bitmask(0, 5);
  static constexpr unsigned int ypos_mask = Common::create_bitmask(6, 11);
  static constexpr unsigned int zpos_mask = Common::create_bitmask(12, 17);
  static constexpr unsigned int normal_mask = Common::create_bitmask(18, 20);
  static constexpr unsigned int uvs_mask = Common::create_bitmask(21, 22);
  static constexpr unsigned int texture_mask = Common::create_bitmask(23, 31);
};

namespace QuadCoord {
  constexpr glm::vec2 bl = glm::vec2(0.f, 0.f);
  constexpr glm::vec2 br = glm::vec2(1.f, 0.f);
  constexpr glm::vec2 tl = glm::vec2(0.f, 1.f);
  constexpr glm::vec2 tr = glm::vec2(1.f, 1.f);
} // namespace QuadCoord

using Message = std::vector<std::uint8_t>;

struct Action {
  struct NewActiveItemData {
    std::optional<Item> item;
  };
  enum Kind {
    new_active_item,
  };
  Kind kind;
  std::any data;

  template <typename T>
  Action(Kind k, const T& obj) : kind(k), data(obj) {}
};

#endif
