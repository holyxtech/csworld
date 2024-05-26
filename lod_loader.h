#ifndef LOD_LOADER_H
#define LOD_LOADER_H

#include <unordered_map>
#include <vector>
#include "chunk.h"
#include "chunk_lod.h"
#include "types.h"

class LodLoader {
public:
  struct Diff {
    enum Kind {
      creation,
    };
    Location location;
    Kind kind;
  };

  void create_lods(const Chunk& chunk);
  bool has_lods(const Location& loc) const;
  const std::vector<Diff>& get_diffs() const;
  void clear_diffs();
  template <LodLevel level>
  const ChunkLod<level> get_lod(const Location& loc) const;
  template <LodLevel level>
  std::array<const ChunkLod<level>*, 6> get_adjacent_lods(const Location& loc) const;

  static constexpr int lod1_sz = 1000;

private:
  struct LodPack {
    ChunkLod<LodLevel::lod1> l1;
    ChunkLod<LodLevel::lod2> l2;

    template <LodLevel level>
    const ChunkLod<level>& get() const;
  };

  std::unordered_map<Location, LodPack, LocationHash> lods_;
  std::unordered_map<Location, int, LocationHash> adjacents_missing_;
  std::vector<Diff> diffs_;
};

#endif