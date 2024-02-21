#ifndef REGION_H
#define REGION_H

#include <memory>
#include <unordered_map>
#include <vector>
#include "chunk.h"

class Region {
public:
  Chunk& get_chunk(Location loc);
  bool has_chunk(Location loc) const;
  void add_chunk(Chunk&& chunk);
  
  static constexpr int max_sz = 256;

private:
  std::unordered_map<Location, Chunk, LocationHash> chunks_;
  
};

#endif // REGION_H
