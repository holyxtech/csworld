#include "region.h"
#include <iostream>
#include "../common.h"

Chunk& Region::get_chunk(Location loc) {
  return chunks_.at(loc);
}

bool Region::has_chunk(Location loc) const {
  return chunks_.contains(loc);
}

void Region::add_chunk(Chunk&& chunk) {
  auto loc = chunk.get_location();

  if (chunks_.size() >= max_sz) {
    int max_difference = 0;
    Chunk* chunk_to_delete = nullptr;

    for (auto& pair : chunks_) {
      auto& chunk = pair.second;
      auto& location = chunk.get_location();
      int difference = LocationMath::difference(loc, location);
      if (difference > max_difference) {
        max_difference = difference;
        chunk_to_delete = &chunk;
      }
    }
    if (chunk_to_delete != nullptr) {
      auto& location = chunk_to_delete->get_location();
      // delete directly...
    }
  }

  chunks_.insert({loc, std::move(chunk)});
}
