#ifndef REGION_H
#define REGION_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tsl/robin_map.h>
#include "chunk.h"
#include "player.h"
#include "section.h"
#include "simobject.h"

class Region {
public:
  struct Diff {
    enum Kind {
      creation,
      deletion,
      water
    };
    Location location;
    Kind kind;
  };

  void add_section(Section section);
  bool has_section(Location2D loc) const;
  std::unordered_map<Location2D, Section, Location2DHash>& get_sections();
  Player& get_player();
  Chunk& get_chunk(Location loc);
  bool has_chunk(Location loc) const;
  std::unordered_map<Location, Chunk, LocationHash>& get_chunks();
  void add_chunk(Chunk&& chunk);
  const std::vector<Diff>& get_diffs() const;
  void clear_diffs();
  Voxel get_voxel(int x, int y, int z) const;
  unsigned char get_lighting(int x, int y, int z) const;
  std::array<Chunk*, 6> get_adjacent_chunks(const Location& loc);
  Section& get_section(const Location2D loc);

  static constexpr int max_sz = 256;

private:
  template <typename Func>
  auto get_data(int x, int y, int z, Func func) const;
  void chunk_to_mesh_generator(const Location& loc);
  void compute_global_lighting(const Location& loc);
  void delete_furthest_chunk(const Location& loc);
  std::array<Location, 6> get_adjacent_locations(const Location& loc) const;

  std::unordered_map<Location2D, Section, Location2DHash> sections_;
  std::unordered_map<Location, Chunk, LocationHash> chunks_;
  std::unordered_set<Location, LocationHash> chunks_sent_;
  std::unordered_map<Location, int, LocationHash> adjacents_missing_;
  std::vector<Diff> diffs_;
  std::vector<SimObject> live_objects_;
  Player player_;

  // has to be at least as big as max_sz
  static constexpr int max_sz_internal = max_sz * 2;
};

#endif // REGION_H
