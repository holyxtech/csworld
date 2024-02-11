#ifndef REGION_H
#define REGION_H

#include <memory>
#include <vector>
#include "chunk.h"

class Region {
public:
  struct Diff {
    enum Kind {
      creation,
      deletion
    };
    const Location& loc;
    Kind kind;
  };

  const Chunk* get_chunk(Location loc) const;
  const std::vector<Chunk>& get_chunks() const;
  void add_chunk(Chunk&& chunk);
  const std::vector<Diff>& get_diffs() const;
  void clear_diffs();

  static constexpr int max_sz = 64;

private:
  void delete_chunk(Chunk& chunk);

  std::vector<Chunk> chunks_;
  std::vector<Diff> diffs_;
};

#endif // REGION_H
