#ifndef REGION_H
#define REGION_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "camera.h"
#include "chunk.h"
#include "player.h"
#include "section.h"

class Region {
public:
  struct Diff {
    enum Kind {
      creation,
      deletion,
    };
    Location location;
    Kind kind;
  };

  Player& get_player();
  const Chunk& get_chunk(const Location& loc) const;
  Chunk& get_chunk(const Location& loc);
  bool has_chunk(const Location& loc) const;
  std::unordered_map<Location, Chunk, LocationHash>& get_chunks();
  void add_chunk(Chunk&& chunk);
  const std::vector<Diff>& get_diffs() const;
  void clear_diffs();
  Voxel get_voxel(int x, int y, int z) const;
  Voxel get_voxel(const Int3D& coord) const;
  void set_voxel(int x, int y, int z, Voxel voxel);
  void set_voxel(const Int3D& coord, Voxel voxel);
  std::array<const Chunk*, 6> get_adjacent_chunks(const Location& loc) const;
  void raycast_place(const glm::dvec3& pos, const glm::dvec3& dir, Voxel voxel, int num_voxels = 50);
  void raycast_remove(const Camera& camera);
  static Location location_from_global_coord(int x, int y, int z);
  static Location location_from_global_coord(const Int3D& coord);
  const std::unordered_set<Location, LocationHash> get_updated_since_reset() const;
  void reset_updated_since_reset();
  bool set_voxel_if_possible(const Location& loc, int idx, Voxel voxel);
  void signal_chunk_update(const Location& loc);

  static std::vector<Int3D> raycast(const glm::dvec3& pos, const glm::dvec3& dir, int num_voxels = 50);
  bool get_first_of_kind_without_obstruction(
    const glm::dvec3& pos, const glm::dvec3& dir, int max_tries, Int3D& coord, Voxel& voxel,
    const std::function<bool(Voxel v)>& kind_test , const std::function<bool(Voxel v)>& obstruction_test) const;
  bool get_until_kind(
    const glm::dvec3& pos, const glm::dvec3& dir, int max_tries, std::vector<Voxel>& voxels,
    const std::function<bool(Voxel v)>& kind_test) const;

  static int max_sz;

private:
  void chunk_to_mesh_generator(const Location& loc);
  void delete_furthest_chunk(const Location& loc);
  std::array<Location, 6> get_adjacent_locations(const Location& loc) const;
  void update_adjacent_chunks(const Int3D& coord);

  std::unordered_map<Location, Chunk, LocationHash> chunks_;
  std::unordered_set<Location, LocationHash> chunks_sent_;
  std::unordered_map<Location, int, LocationHash> adjacents_missing_;
  std::vector<Diff> diffs_;
  Player player_;
  std::unordered_set<Location, LocationHash> updated_since_reset_;

  // has to be at least as big as max_sz
  static int max_sz_internal;
};

#endif // REGION_H
