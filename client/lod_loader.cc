#include "lod_loader.h"

bool LodLoader::has_lods(const Location& loc) const {
  return lods_.contains(loc);
}

void LodLoader::create_lods(const Chunk& chunk) {
  auto& loc = chunk.get_location();
  auto& voxels = chunk.get_voxels();
  auto l1 = ChunkLod<LodLevel::lod1>(voxels);
  auto lod_pack = LodPack{std::move(l1)};
  lods_.insert({loc, std::move(lod_pack)});

  auto adjacent = LocationMath::get_adjacent_locations(loc);

  for (auto& location : adjacent) {
    if (!adjacents_missing_.contains(location))
      adjacents_missing_.insert({location, 5});
    else
      --adjacents_missing_[location];

    if (adjacents_missing_[location] == 0 && lods_.contains(location)) {
      diffs_.emplace_back(location, Diff::creation);
    }
  }

  if (!adjacents_missing_.contains(loc)) {
    adjacents_missing_.insert({loc, 6});
  } else if (
    adjacents_missing_[loc] == 0) {
    diffs_.emplace_back(loc, Diff::creation);
  }
}

const std::vector<LodLoader::Diff>& LodLoader::get_diffs() const {
  return diffs_;
}
void LodLoader::clear_diffs() {
  diffs_.clear();
}

template <LodLevel level>
std::array<const ChunkLod<level>*, 6> LodLoader::get_adjacent_lods(const Location& loc) const {
  return std::array<const ChunkLod<level>*, 6>{
    &lods_.at(Location{loc[0] - 1, loc[1], loc[2]}).get<level>(),
    &lods_.at(Location{loc[0] + 1, loc[1], loc[2]}).get<level>(),
    &lods_.at(Location{loc[0], loc[1] - 1, loc[2]}).get<level>(),
    &lods_.at(Location{loc[0], loc[1] + 1, loc[2]}).get<level>(),
    &lods_.at(Location{loc[0], loc[1], loc[2] - 1}).get<level>(),
    &lods_.at(Location{loc[0], loc[1], loc[2] + 1}).get<level>()};
}

template std::array<const ChunkLod<LodLevel::lod1>*, 6> LodLoader::get_adjacent_lods<LodLevel::lod1>(const Location& loc) const;
template std::array<const ChunkLod<LodLevel::lod2>*, 6> LodLoader::get_adjacent_lods<LodLevel::lod2>(const Location& loc) const;

template <LodLevel level>
const ChunkLod<level>& LodLoader::LodPack::get() const {
  if constexpr (level == LodLevel::lod1)
    return l1;
  else if constexpr (level == LodLevel::lod2)
    return l2;
}

template <LodLevel level>
const ChunkLod<level>& LodLoader::get_lod(const Location& loc) const {
  if constexpr (level == LodLevel::lod1)
    return lods_.at(loc).l1;
  else if constexpr (level == LodLevel::lod2)
    return lods_.at(loc).l2;
}

template const ChunkLod<LodLevel::lod1>& LodLoader::get_lod<LodLevel::lod1>(const Location& loc) const;
template const ChunkLod<LodLevel::lod2>& LodLoader::get_lod<LodLevel::lod2>(const Location& loc) const;