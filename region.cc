#include "region.h"
#include <iostream>

const std::vector<Chunk>& Region::get_chunks() const {
  return chunks_;
}

const Chunk* Region::get_chunk(Location loc) const {

  for (auto& chunk : chunks_) {
    if (chunk.get_location() == loc)
      return &chunk;
  }
  return nullptr;
}

void Region::add_chunk(Chunk&& chunk) {
  auto& loc = chunk.get_location();

  if (chunks_.size() >= max_sz) {
    int max_difference = 0;
    Chunk* chunk_to_delete = nullptr;
    for (auto& chunk_ : chunks_) {
      auto& location = chunk_.get_location();
      int difference = LocationMath::difference(loc, location);
      if (difference > max_difference) {
        difference = max_difference;
        chunk_to_delete = &chunk_;
      }
    }
    if (chunk_to_delete != nullptr)
      delete_chunk(*chunk_to_delete);
  }

  chunks_.emplace_back(std::move(chunk));
  diffs_.emplace_back(Diff{chunks_.back().get_location(), Diff::creation});
}

void Region::delete_chunk(Chunk& chunk) {
  auto& loc = chunk.get_location();
  diffs_.emplace_back(Diff{loc, Diff::deletion});
}

const std::vector<Region::Diff>& Region::get_diffs() const {
  return diffs_;
}

void Region::clear_diffs() {
  // process deletions...
  diffs_.clear();
}