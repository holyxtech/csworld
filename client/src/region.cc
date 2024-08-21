#include "region.h"
#include <array>
#include <cfloat>
#include <chrono>
#include <iostream>
#include <optional>
#include <queue>

int Region::max_sz = 512;
int Region::max_sz_internal = Region::max_sz * 2;

std::unordered_map<Location, Chunk, LocationHash>& Region::get_chunks() {
  return chunks_;
}

const Chunk& Region::get_chunk(const Location& loc) const {
  return chunks_.at(loc);
}
Chunk& Region::get_chunk(const Location& loc) {
  return chunks_.at(loc);
}

bool Region::has_chunk(const Location& loc) const {
  return chunks_.contains(loc);
}

std::array<const Chunk*, 6> Region::get_adjacent_chunks(const Location& loc) const {
  return std::array<const Chunk*, 6>{
    &chunks_.at(Location{loc[0] - 1, loc[1], loc[2]}),
    &chunks_.at(Location{loc[0] + 1, loc[1], loc[2]}),
    &chunks_.at(Location{loc[0], loc[1] - 1, loc[2]}),
    &chunks_.at(Location{loc[0], loc[1] + 1, loc[2]}),
    &chunks_.at(Location{loc[0], loc[1], loc[2] - 1}),
    &chunks_.at(Location{loc[0], loc[1], loc[2] + 1})};
}

std::array<Location, 6> Region::get_adjacent_locations(const Location& loc) const {
  return std::array{
    Location{loc[0] - 1, loc[1], loc[2]},
    Location{loc[0] + 1, loc[1], loc[2]},
    Location{loc[0], loc[1] - 1, loc[2]},
    Location{loc[0], loc[1] + 1, loc[2]},
    Location{loc[0], loc[1], loc[2] - 1},
    Location{loc[0], loc[1], loc[2] + 1}};
}

void Region::delete_furthest_chunk(const Location& loc) {
  if (chunks_sent_.size() >= max_sz) {
    int max_difference = 0;
    Chunk* chunk_to_delete = nullptr;
    const Location* to_delete;
    for (auto& location : chunks_sent_) {
      auto& chunk = chunks_.at(location);
      if (chunk.check_flag(ChunkFlags::Deleted)) {
        continue;
      }

      int difference = LocationMath::distance(loc, location);
      if (difference > max_difference) {
        max_difference = difference;
        chunk_to_delete = &chunk;
        to_delete = &location;
      }
    }
    chunk_to_delete->set_flag(ChunkFlags::Deleted);
    diffs_.emplace_back(*to_delete, Diff::deletion);
    chunks_sent_.erase(*to_delete);
  }
}

void Region::chunk_to_mesh_generator(const Location& loc) {
  delete_furthest_chunk(loc);
  diffs_.emplace_back(loc, Diff::creation);
  chunks_sent_.insert(loc);
}

void Region::add_chunk(Chunk&& chunk) {
  auto loc = chunk.get_location();
  chunks_.insert({loc, std::move(chunk)});

  auto adjacent = get_adjacent_locations(loc);

  for (auto& location : adjacent) {
    if (!adjacents_missing_.contains(location))
      adjacents_missing_.insert({location, 5});
    else
      --adjacents_missing_[location];

    if (adjacents_missing_[location] == 0 &&
        chunks_.contains(location) &&
        !chunks_sent_.contains(location) &&
        !chunks_.at(location).check_flag(ChunkFlags::Deleted) &&
        !chunks_.at(location).check_flag(ChunkFlags::Empty)) {
      chunk_to_mesh_generator(location);
    }
  }

  if (!adjacents_missing_.contains(loc)) {
    adjacents_missing_.insert({loc, 6});
  } else if (
    adjacents_missing_[loc] == 0 &&
    !chunks_.at(loc).check_flag(ChunkFlags::Empty)) {
    chunk_to_mesh_generator(loc);
  }
}

const std::vector<Region::Diff>& Region::get_diffs() const {
  return diffs_;
}

void Region::clear_diffs() {
  for (auto& diff : diffs_) {
    auto& loc = diff.location;
    if (diff.kind == Region::Diff::deletion) {
      chunks_.erase(loc);
      // std::cout<<"deleting at "<<loc<<std::endl;
      auto adjacent = get_adjacent_locations(loc);
      for (auto& location : adjacent) {
        ++adjacents_missing_[location];
      }
    }
  }

  if (chunks_.size() > max_sz_internal) {
    std::vector<Location> loaded_locations;
    for (auto& [location, _] : chunks_) {
      if (!chunks_sent_.contains(location))
        loaded_locations.push_back(location);
    }

    auto& player_location_ = player_.get_last_location();
    std::sort(
      loaded_locations.begin(), loaded_locations.end(),
      [&player_location_](const Location l1, const Location l2) {
        auto d1 = LocationMath::distance(l1, player_location_);
        auto d2 = LocationMath::distance(l2, player_location_);
        return d1 > d2;
      });
    int to_remove = chunks_.size() - max_sz_internal;

    for (int i = 0; i < to_remove; ++i) {
      auto& location = loaded_locations[i];
      chunks_.erase(location);
      // std::cout<<"removing at "<<location<<std::endl;

      auto adjacent = get_adjacent_locations(loaded_locations[i]);
      for (auto& location : adjacent) {
        ++adjacents_missing_[location];
      }
    }
  }

  diffs_.clear();
}

Location Region::location_from_global_coord(int x, int y, int z) {
  return location_from_global_coord(Int3D{x, y, z});
}
Location Region::location_from_global_coord(const Int3D& coord) {
  return Location{
    static_cast<int>(std::floor(static_cast<double>(coord[0]) / Chunk::sz_x)),
    static_cast<int>(std::floor(static_cast<double>(coord[1]) / Chunk::sz_y)),
    static_cast<int>(std::floor(static_cast<double>(coord[2]) / Chunk::sz_z)),
  };
}

Voxel Region::get_voxel(const Int3D& coord) const {
  auto location = location_from_global_coord(coord);
  const auto& chunk = chunks_.at(location);
  return chunk.get_voxel(
    ((coord[0] % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x,
    ((coord[1] % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y,
    ((coord[2] % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z);
}

Voxel Region::get_voxel(int x, int y, int z) const {
  return get_voxel(Int3D{x, y, z});
}

void Region::set_voxel(int x, int y, int z, Voxel voxel) {
  set_voxel(Int3D{x, y, z}, voxel);
}

void Region::set_voxel(const Int3D& coord, Voxel voxel) {
  auto location = location_from_global_coord(coord);
  auto& chunk = chunks_.at(location);
  return chunk.set_voxel(
    ((coord[0] % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x,
    ((coord[1] % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y,
    ((coord[2] % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z,
    voxel);
}

Player& Region::get_player() {
  return player_;
}

// Credit: https://github.com/francisengelmann/fast_voxel_traversal
std::vector<Int3D> Region::raycast(const glm::dvec3& pos, const glm::dvec3& dir, int num_voxels) {
  std::vector<Int3D> visited_voxels;
  visited_voxels.reserve(num_voxels);

  Int3D current_voxel{
    static_cast<int>(std::floor(pos[0])),
    static_cast<int>(std::floor(pos[1])),
    static_cast<int>(std::floor(pos[2]))};

  double stepX = (dir[0] >= 0) ? 1 : -1;
  double stepY = (dir[1] >= 0) ? 1 : -1;
  double stepZ = (dir[2] >= 0) ? 1 : -1;

  double next_voxel_boundary_x = (current_voxel[0] + stepX);
  double next_voxel_boundary_y = (current_voxel[1] + stepY);
  double next_voxel_boundary_z = (current_voxel[2] + stepZ);

  if (stepX < 0)
    ++next_voxel_boundary_x;
  if (stepY < 0)
    ++next_voxel_boundary_y;
  if (stepZ < 0)
    ++next_voxel_boundary_z;
  double tMaxX = (dir[0] != 0) ? (next_voxel_boundary_x - pos[0]) / dir[0] : DBL_MAX;
  double tMaxY = (dir[1] != 0) ? (next_voxel_boundary_y - pos[1]) / dir[1] : DBL_MAX;
  double tMaxZ = (dir[2] != 0) ? (next_voxel_boundary_z - pos[2]) / dir[2] : DBL_MAX;

  double tDeltaX = (dir[0] != 0) ? 1 / dir[0] * stepX : DBL_MAX;
  double tDeltaY = (dir[1] != 0) ? 1 / dir[1] * stepY : DBL_MAX;
  double tDeltaZ = (dir[2] != 0) ? 1 / dir[2] * stepZ : DBL_MAX;

  visited_voxels.push_back(current_voxel);

  int i = 0;
  while (i++ < num_voxels - 1) {
    if (tMaxX < tMaxY) {
      if (tMaxX < tMaxZ) {
        current_voxel[0] += stepX;
        tMaxX += tDeltaX;
      } else {
        current_voxel[2] += stepZ;
        tMaxZ += tDeltaZ;
      }
    } else {
      if (tMaxY < tMaxZ) {
        current_voxel[1] += stepY;
        tMaxY += tDeltaY;
      } else {
        current_voxel[2] += stepZ;
        tMaxZ += tDeltaZ;
      }
    }
    visited_voxels.push_back(current_voxel);
  }
  return visited_voxels;
}

void Region::update_adjacent_chunks(const Int3D& coord) {
  std::unordered_set<Location, LocationHash> dirty;
  auto loc = location_from_global_coord(coord);
  auto local = Chunk::to_local(coord);
  if (local[0] == 0) {
    dirty.insert(Location{loc[0] - 1, loc[1], loc[2]});
  } else if (local[0] == Chunk::sz_x - 1) {
    dirty.insert(Location{loc[0] + 1, loc[1], loc[2]});
  }
  if (local[1] == 0) {
    dirty.insert(Location{loc[0], loc[1] - 1, loc[2]});
  } else if (local[1] == Chunk::sz_y - 1) {
    dirty.insert(Location{loc[0], loc[1] + 1, loc[2]});
  }
  if (local[2] == 0) {
    dirty.insert(Location{loc[0], loc[1], loc[2] - 1});
  } else if (local[2] == Chunk::sz_z - 1) {
    dirty.insert(Location{loc[0], loc[1], loc[2] + 1});
  }
  for (auto& loc : dirty) {
    if (chunks_sent_.contains(loc) && adjacents_missing_[loc] == 0) {
      diffs_.emplace_back(loc, Diff::creation);
    }
  }
}

bool Region::get_first_of_kind_without_obstruction(
  const glm::dvec3& pos, const glm::dvec3& dir, int max_tries, Int3D& coord, Voxel& voxel,
  const std::function<bool(Voxel v)>& kind_test, const std::function<bool(Voxel v)>& obstruction_test) const {
  auto visited = raycast(pos, dir, max_tries);
  for (auto& v : visited) {
    auto loc = location_from_global_coord(v);
    if (!chunks_.contains(loc))
      return false;

    auto voxel_at_v = get_voxel(v);
    if (kind_test(voxel_at_v)) {
      voxel = voxel_at_v;
      coord = v;
      return true;
    } else if (obstruction_test(voxel_at_v)) {
      return false;
    }
  }
  return false;
}

bool Region::get_until_kind(
  const glm::dvec3& pos, const glm::dvec3& dir, int max_tries, std::vector<Voxel>& voxels,
  const std::function<bool(Voxel v)>& kind_test) const {
  auto visited = raycast(pos, dir, max_tries);
  for (auto& v : visited) {
    auto loc = location_from_global_coord(v);
    if (!chunks_.contains(loc))
      return false;
    auto voxel_at_v = get_voxel(v);
    if (kind_test(voxel_at_v)) {
      return true;
    }
    voxels.push_back(voxel_at_v);
  }
  return false;
}

void Region::raycast_place(const glm::dvec3& pos, const glm::dvec3& dir, Voxel voxel, int num_voxels) {
  auto visited = raycast(pos, dir, num_voxels);
  for (int i = 0; i < visited.size(); ++i) {
    auto coord = visited[i];

    auto loc = location_from_global_coord(coord);
    if (chunks_.contains(loc)) {
      auto& chunk = chunks_.at(loc);
      auto local = Chunk::to_local(coord);
      auto voxel_at_position = chunk.get_voxel(local[0], local[1], local[2]);
      if (voxel_at_position != Voxel::empty) {
        if (i == 0)
          return;
        auto coord = visited[i - 1];
        auto loc = location_from_global_coord(coord);
        if (chunks_.contains(loc) && adjacents_missing_[loc] == 0 &&
            !chunks_.at(loc).check_flag(ChunkFlags::Deleted)) {
          auto& chunk = chunks_.at(loc);
          auto local = Chunk::to_local(coord);
          chunk.set_voxel(local[0], local[1], local[2], voxel);
          updated_since_reset_.insert(loc);

          if (!chunks_sent_.contains(loc)) {
            delete_furthest_chunk(loc);
            chunks_sent_.insert(loc);
          }
          diffs_.emplace_back(loc, Diff::creation);

          update_adjacent_chunks(coord);
        }

        break;
      }
    }
  }
}

void Region::raycast_remove(const Camera& camera) {
  auto visited = raycast(camera.get_position(), camera.get_front());
  for (int i = 0; i < visited.size(); ++i) {
    auto coord = visited[i];
    auto loc = location_from_global_coord(coord);
    if (chunks_sent_.contains(loc) && adjacents_missing_[loc] == 0) {
      auto& chunk = chunks_.at(loc);
      auto local = Chunk::to_local(coord);
      auto voxel = chunk.get_voxel(local[0], local[1], local[2]);
      if (voxel != Voxel::empty) {
        chunk.set_voxel(local[0], local[1], local[2], Voxel::empty);
        updated_since_reset_.insert(loc);
        diffs_.emplace_back(loc, Diff::creation);
        update_adjacent_chunks(coord);
        break;
      }
    }
  }
}

const std::unordered_set<Location, LocationHash> Region::get_updated_since_reset() const {
  return updated_since_reset_;
}

void Region::reset_updated_since_reset() {
  updated_since_reset_.clear();
}

void Region::signal_chunk_update(const Location& loc) {
  if (!chunks_.contains(loc))
    return;
  auto& chunk = chunks_.at(loc);
  if (chunk.check_flag(ChunkFlags::Deleted))
    return;
  chunk.unset_flag(ChunkFlags::Empty);
  if (adjacents_missing_[loc] != 0)
    return;
  if (!chunks_sent_.contains(loc)) {
    chunk_to_mesh_generator(loc);    
  } else {
    diffs_.emplace_back(loc, Diff::creation);
  }
  updated_since_reset_.insert(loc);
}

bool Region::set_voxel_if_possible(const Location& loc, int idx, Voxel voxel) {
  auto it = chunks_.find(loc);
  if (it == chunks_.end())
    return false;
  auto& chunk = it->second;
  chunk.set_voxel(idx, voxel);
  return true;
}