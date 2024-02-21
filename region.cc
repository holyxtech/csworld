#include "region.h"
#include <iostream>

void Region::add_section(Section section) {
  sections_.insert({section.location, section});
}

bool Region::has_section(Location2D loc) const {
  return sections_.contains(loc);
}

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

  diffs_.emplace_back(Diff{loc, Diff::water});
  diffs_.emplace_back(Diff{loc, Diff::creation});
  chunks_.insert({loc, std::move(chunk)});
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