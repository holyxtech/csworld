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
      if (chunk_.check_flag(Chunk::Flags::DELETED))
        continue;
      auto& location = chunk_.get_location();
      int difference = LocationMath::difference(loc, location);
      if (difference > max_difference) {
        max_difference = difference;
        chunk_to_delete = &chunk_;
      }
    }
    if (chunk_to_delete != nullptr) {
      auto& location = chunk_to_delete->get_location();
      diffs_.emplace_back(Diff{location, Diff::deletion});
      chunk_to_delete->set_flag(Chunk::Flags::DELETED);
    }
  }

  chunks_.emplace_back(std::move(chunk));
  diffs_.emplace_back(Diff{chunks_.back().get_location(), Diff::creation});
}

const std::vector<Region::Diff>& Region::get_diffs() const {
  return diffs_;
}

void Region::clear_diffs() {
  // process deletions...
  // TODO: delete all chunks...

  diffs_.clear();
}