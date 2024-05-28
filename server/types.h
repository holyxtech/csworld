#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <utility>
#include <vector>
#include "../common.h"

using Location = std::array<int, 3>;
using Location2D = std::array<int, 2>;

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

using Message = std::vector<uint8_t>;
struct MessageWithId {
  Message message;
  int id;
};

struct hash_pair final {
  template <class TFirst, class TSecond>
  size_t operator()(const std::pair<TFirst, TSecond>& p) const noexcept {
    uintmax_t hash = std::hash<TFirst>{}(p.first);
    hash <<= sizeof(uintmax_t) * 4;
    hash ^= std::hash<TSecond>{}(p.second);
    return std::hash<uintmax_t>{}(hash);
  }
};

struct Section {
  Location2D location;
  int elevation;
  std::array<Common::LandCover, Common::landcover_tiles_per_sector> landcover;
};

#endif
