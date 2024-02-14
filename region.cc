#include "region.h"
#include <iostream>

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
      if (chunk.check_flag(Chunk::Flags::DELETED))
        continue;

      auto& location = chunk.get_location();
      int difference = LocationMath::difference(loc, location);
      if (difference > max_difference) {
        max_difference = difference;
        chunk_to_delete = &chunk;
      }
    }
    if (chunk_to_delete != nullptr) {
      auto& location = chunk_to_delete->get_location();
      diffs_.emplace_back(Diff{location, Diff::deletion});
      chunk_to_delete->set_flag(Chunk::Flags::DELETED);
    }
  }

  chunks_.insert({loc, std::move(chunk)});
  diffs_.emplace_back(Diff{loc, Diff::creation});
}

const std::vector<Region::Diff>& Region::get_diffs() const {
  return diffs_;
}

void Region::clear_diffs() {
  for (auto& diff : diffs_) {
    auto& loc = diff.location;
    if (diff.kind == Region::Diff::deletion) {
      chunks_.erase(loc);
    }
  }

  diffs_.clear();
}