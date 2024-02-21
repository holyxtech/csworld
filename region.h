#ifndef REGION_H
#define REGION_H

#include <memory>
#include <unordered_map>
#include <vector>
#include "chunk.h"
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

  Chunk& get_chunk(Location loc);
  bool has_chunk(Location loc) const;
  const std::vector<Chunk>& get_chunks() const;
  void add_chunk(Chunk&& chunk);
  const std::vector<Diff>& get_diffs() const;
  void clear_diffs();

  static constexpr int max_sz = 256;

private:
  std::unordered_map<Location2D, Section, Location2DHash> sections_;
  std::unordered_map<Location, Chunk, LocationHash> chunks_;
  std::vector<Diff> diffs_;

  std::vector<SimObject> live_objects_;
};

#endif // REGION_H
