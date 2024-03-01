#include "region.h"
#include <array>
#include <iostream>

void Region::add_section(Section section) {
  sections_.insert({section.get_location(), section});
}

bool Region::has_section(Location2D loc) const {
  return sections_.contains(loc);
}

std::unordered_map<Location2D, Section, Location2DHash>& Region::get_sections() {
  return sections_;
}

Chunk& Region::get_chunk(Location loc) {
  return chunks_.at(loc);
}

bool Region::has_chunk(Location loc) const {
  return chunks_.contains(loc);
}

// make sure to never request more chunks than there is space for them (so you don't end up with nothing to delete here)
void Region::delete_furthest_chunk(Location& loc) {
  if (chunks_sent_.size() >= max_sz) {
    int max_difference = 0;
    Chunk* chunk_to_delete = nullptr;
    const Location* to_delete;
    for (auto& location : chunks_sent_) {
      auto& chunk = chunks_.at(location);
      if (chunk.check_flag(Chunk::Flags::DELETED)) {
        continue;
      }

      int difference = LocationMath::distance(loc, location);
      if (difference > max_difference) {
        max_difference = difference;
        chunk_to_delete = &chunk;
        to_delete = &location;
      }
    }

    chunk_to_delete->set_flag(Chunk::Flags::DELETED);
    diffs_.emplace_back(Diff{*to_delete, Diff::deletion});
    chunks_sent_.erase(*to_delete);
  }
}

void Region::add_chunk(Chunk&& chunk) {
  auto loc = chunk.get_location();
  chunks_.insert({loc, std::move(chunk)});
  std::array<Location, 6> arr = {
    Location{loc[0] + 1, loc[1], loc[2]},
    Location{loc[0] - 1, loc[1], loc[2]},
    Location{loc[0], loc[1] + 1, loc[2]},
    Location{loc[0], loc[1] - 1, loc[2]},
    Location{loc[0], loc[1], loc[2] + 1},
    Location{loc[0], loc[1], loc[2] - 1},
  };

  for (auto& location : arr) {
    if (!adjacents_missing_.contains(location)) {
      adjacents_missing_.insert({location, 6});
    }

    --adjacents_missing_[location];

    if (adjacents_missing_[location] == 0 &&
        chunks_.contains(location) &&
        !chunks_sent_.contains(location) &&
        !chunks_.at(location).check_flag(Chunk::Flags::DELETED)) {
      delete_furthest_chunk(location);
      diffs_.emplace_back(Diff{location, Diff::water});
      diffs_.emplace_back(Diff{location, Diff::creation});
      chunks_sent_.insert(location);
    }
  }

  if (!adjacents_missing_.contains(loc)) {
    adjacents_missing_.insert({loc, 6});
  } else if (adjacents_missing_[loc] == 0) {
    delete_furthest_chunk(loc);
    diffs_.emplace_back(Diff{loc, Diff::water});
    diffs_.emplace_back(Diff{loc, Diff::creation});
    chunks_sent_.insert(loc);
  }
}

const std::vector<Region::Diff>& Region::get_diffs() const {
  return diffs_;
}

void Region::clear_diffs() {
  for (auto& diff : diffs_) {
    auto& loc = diff.location;
    if (diff.kind == Region::Diff::deletion) {
      // keep in mind, that if a chunk is never loaded in (because its neighbours never materialised)
      // then this will never be called
      // so it will be necessary to occasionally look for distant chunks and erase them.
      chunks_.erase(loc);
      sections_.erase(Location2D{loc[0], loc[2]});
      std::array<Location, 6> arr = {
        Location{loc[0] + 1, loc[1], loc[2]},
        Location{loc[0] - 1, loc[1], loc[2]},
        Location{loc[0], loc[1] + 1, loc[2]},
        Location{loc[0], loc[1] - 1, loc[2]},
        Location{loc[0], loc[1], loc[2] + 1},
        Location{loc[0], loc[1], loc[2] - 1},
      };
      for (auto& location : arr) {
        ++adjacents_missing_[location];
      }
    }
  }
  diffs_.clear();
}

// Presumes that the chunk exists
Voxel::VoxelType Region::get_voxel(int x, int y, int z) const {
  auto location = Location{
    static_cast<int>(std::floor(static_cast<float>(x) / Chunk::sz_x)),
    static_cast<int>(std::floor(static_cast<float>(y) / Chunk::sz_y)),
    static_cast<int>(std::floor(static_cast<float>(z) / Chunk::sz_z)),
  };
  auto& chunk = chunks_.at(location);
  // std::cout << "mods: " << x % Chunk::sz_x << "," << y % Chunk::sz_y << "," << z % Chunk::sz_z << std::endl;
  return chunk.get_voxel(
    ((x % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x,
    ((y % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y,
    ((z % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z);
}