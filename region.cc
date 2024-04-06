#include "region.h"
#include <array>
#include <cfloat>
#include <chrono>
#include <iostream>
#include <optional>
#include <queue>

std::unordered_map<Location, Chunk, LocationHash>& Region::get_chunks() {
  return chunks_;
}

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

void Region::delete_furthest_chunk(const Location& loc) {
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

std::array<Chunk*, 6> Region::get_adjacent_chunks(const Location& loc) {
  return std::array<Chunk*, 6>{
    &chunks_.at(Location{loc[0] - 1, loc[1], loc[2]}),
    &chunks_.at(Location{loc[0] + 1, loc[1], loc[2]}),
    &chunks_.at(Location{loc[0], loc[1] - 1, loc[2]}),
    &chunks_.at(Location{loc[0], loc[1] + 1, loc[2]}),
    &chunks_.at(Location{loc[0], loc[1], loc[2] - 1}),
    &chunks_.at(Location{loc[0], loc[1], loc[2] + 1})};
}

std::array<Location, 6> Region::get_adjacent_locations(const Location& loc) const {
  return std::array<Location, 6>{
    Location{loc[0] - 1, loc[1], loc[2]},
    Location{loc[0] + 1, loc[1], loc[2]},
    Location{loc[0], loc[1] - 1, loc[2]},
    Location{loc[0], loc[1] + 1, loc[2]},
    Location{loc[0], loc[1], loc[2] - 1},
    Location{loc[0], loc[1], loc[2] + 1},
  };
}

void Region::compute_global_lighting(const Location& loc) {
  auto& chunk = chunks_.at(loc);
  std::queue<Int3D> lights;
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int y = 0; y < Chunk::sz_y; ++y) {
      if (chunk.get_voxel(0, y, z) < Voxel::OPAQUE_LOWER)
        lights.emplace(Int3D{loc[0] * Chunk::sz_x, y + loc[1] * Chunk::sz_y, z + loc[2] * Chunk::sz_z});
      if (chunk.get_voxel(Chunk::sz_x - 1, y, z) < Voxel::OPAQUE_LOWER)
        lights.emplace(Int3D{(loc[0] + 1) * Chunk::sz_x - 1, y + loc[1] * Chunk::sz_y, z + loc[2] * Chunk::sz_z});
    }
  }
  for (int z = 0; z < Chunk::sz_z; ++z) {
    for (int x = 0; x < Chunk::sz_x; ++x) {
      if (chunk.get_voxel(x, 0, z) < Voxel::OPAQUE_LOWER)
        lights.emplace(Int3D{x + loc[0] * Chunk::sz_x, loc[1] * Chunk::sz_y, z + loc[2] * Chunk::sz_z});
      if (chunk.get_voxel(x, Chunk::sz_y - 1, z) < Voxel::OPAQUE_LOWER)
        lights.emplace(Int3D{x + loc[0] * Chunk::sz_x, (loc[1] + 1) * Chunk::sz_y - 1, z + loc[2] * Chunk::sz_z});
    }
  }
  for (int y = 0; y < Chunk::sz_y; ++y) {
    for (int x = 0; x < Chunk::sz_x; ++x) {
      if (chunk.get_voxel(x, y, 0) < Voxel::OPAQUE_LOWER)
        lights.emplace(Int3D{x + loc[0] * Chunk::sz_x, y + loc[1] * Chunk::sz_y, loc[2] * Chunk::sz_z});
      if (chunk.get_voxel(x, y, Chunk::sz_z - 1) < Voxel::OPAQUE_LOWER)
        lights.emplace(Int3D{x + loc[0] * Chunk::sz_x, y + loc[1] * Chunk::sz_y, (loc[2] + 1) * Chunk::sz_z - 1});
    }
  }

  auto adjacent_chunks = get_adjacent_chunks(loc);

  auto get_containing_chunk = [this, &loc, &chunk, &adjacent_chunks](Int3D& coord) {
    int x_loc = static_cast<int>(std::floor(static_cast<double>(coord[0]) / Chunk::sz_x));
    int y_loc = static_cast<int>(std::floor(static_cast<double>(coord[1]) / Chunk::sz_y));
    int z_loc = static_cast<int>(std::floor(static_cast<double>(coord[2]) / Chunk::sz_z));

    std::optional<Chunk*> containing_chunk;

    int x_diff = x_loc - loc[0];
    int y_diff = y_loc - loc[1];
    int z_diff = z_loc - loc[2];
    int sum = std::abs(x_diff) + std::abs(y_diff) + std::abs(z_diff);
    if (sum == 0) {
      containing_chunk = &chunk;
    } else if (sum == 1) {
      if (x_diff == -1)
        containing_chunk = adjacent_chunks[Direction::nx];
      else if (x_diff == 1)
        containing_chunk = adjacent_chunks[Direction::px];
      else if (y_diff == -1)
        containing_chunk = adjacent_chunks[Direction::ny];
      else if (y_diff == 1)
        containing_chunk = adjacent_chunks[Direction::py];
      else if (z_diff == -1)
        containing_chunk = adjacent_chunks[Direction::nz];
      else if (z_diff == 1)
        containing_chunk = adjacent_chunks[Direction::pz];
    } else if (chunks_.contains(Location{x_loc, y_loc, z_loc})) {
      containing_chunk = &chunks_.at({Location{x_loc, y_loc, z_loc}});
    }
    return containing_chunk;
  };

  std::array<std::tuple<int, int, int>, 5> coord_flip =
    {std::make_tuple(1, 0, 0),
     std::make_tuple(-1, 0, 0),
     std::make_tuple(0, 1, 0),
     std::make_tuple(0, 0, 1),
     std::make_tuple(0, 0, -1)};

  std::unordered_set<Location, LocationHash> dirty;

  while (!lights.empty()) {
    auto& coord = lights.front();
    Chunk& origin_chunk = *get_containing_chunk(coord).value();
    auto local_origin = Chunk::to_local(coord);
    auto lighting = origin_chunk.get_lighting(local_origin[0], local_origin[1], local_origin[2]);

    // nx ,px, nz, pz, py
    for (auto& flip : coord_flip) {
      Int3D global{coord[0] + std::get<0>(flip), coord[1] + std::get<1>(flip), coord[2] + std::get<2>(flip)};
      std::optional<Chunk*> containing_chunk_opt = get_containing_chunk(global);
      if (containing_chunk_opt.has_value()) {
        Chunk& containing_chunk = *containing_chunk_opt.value();
        auto local = Chunk::to_local(global);
        if (containing_chunk.get_voxel(local[0], local[1], local[2]) < Voxel::OPAQUE_LOWER) {
          auto adj_lighting = containing_chunk.get_lighting(local[0], local[1], local[2]);
          if (lighting + 1 < adj_lighting) {
            origin_chunk.set_lighting(local_origin[0], local_origin[1], local_origin[2], adj_lighting - 1);
            lights.emplace(coord);
          } else if (lighting > adj_lighting + 1) {
            containing_chunk.set_lighting(local[0], local[1], local[2], lighting - 1);
            dirty.insert(containing_chunk.get_location());
            lights.emplace(global);
          }
        }
      }
    }

    // ny
    Int3D global{coord[0], coord[1] - 1, coord[2]};
    std::optional<Chunk*> containing_chunk_opt = get_containing_chunk(global);
    if (containing_chunk_opt.has_value()) {
      Chunk& containing_chunk = *containing_chunk_opt.value();
      auto local = Chunk::to_local(global);
      auto v = containing_chunk.get_voxel(local[0], local[1], local[2]);
      if (v < Voxel::OPAQUE_LOWER) {
        auto adj_lighting = containing_chunk.get_lighting(local[0], local[1], local[2]);
        if (lighting + 1 < adj_lighting) {
          origin_chunk.set_lighting(local_origin[0], local_origin[1], local_origin[2], adj_lighting - 1);
          lights.emplace(coord);
        } else if (lighting > adj_lighting) {
          if (v < Voxel::PARTIAL_OPAQUE_LOWER) {
            containing_chunk.set_lighting(local[0], local[1], local[2], lighting);
          } else {
            containing_chunk.set_lighting(local[0], local[1], local[2], lighting - 1);
          }
          dirty.insert(containing_chunk.get_location());
          lights.emplace(global);
        }
      }
    }

    lights.pop();
  }
  for (auto& location : dirty) {
    if (adjacents_missing_[location] == 0 && location != loc &&
        chunks_sent_.contains(location)) {
      diffs_.emplace_back(Diff{location, Diff::creation});
    }
  }
}

void Region::chunk_to_mesh_generator(const Location& loc) {
  delete_furthest_chunk(loc);
  compute_global_lighting(loc);
  diffs_.emplace_back(Diff{loc, Diff::creation});
  chunks_sent_.insert(loc);
}

void Region::add_chunk(Chunk&& chunk) {
  auto loc = chunk.get_location();

  chunk.compute_lighting(sections_.at(Location2D{loc[0], loc[2]}));

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
        !chunks_.at(location).check_flag(Chunk::Flags::DELETED) &&
        chunks_.at(location).check_flag(Chunk::Flags::NONEMPTY)) {
      chunk_to_mesh_generator(location);
    }
  }

  if (!adjacents_missing_.contains(loc)) {
    adjacents_missing_.insert({loc, 6});
  } else if (
    adjacents_missing_[loc] == 0 &&
    chunks_.at(loc).check_flag(Chunk::Flags::NONEMPTY)) {
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
      sections_.erase(Location2D{loc[0], loc[2]});

      auto adjacent = get_adjacent_locations(loc);
      for (auto& location : adjacent) {
        ++adjacents_missing_[location];
      }
    }
  }

  if (chunks_.size() > max_sz_internal) {
    std::vector<Location> loaded_locations;
    for (auto& pair : chunks_) {
      if (!chunks_sent_.contains(pair.first))
        loaded_locations.emplace_back(pair.first);
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
      sections_.erase(Location2D{location[0], location[2]});
      auto adjacent = get_adjacent_locations(loaded_locations[i]);
      for (auto& location : adjacent) {
        ++adjacents_missing_[location];
      }
    }
  }

  diffs_.clear();
}

Location Region::location_from_global_coords(int x, int y, int z) {
  return Location{
    static_cast<int>(std::floor(static_cast<double>(x) / Chunk::sz_x)),
    static_cast<int>(std::floor(static_cast<double>(y) / Chunk::sz_y)),
    static_cast<int>(std::floor(static_cast<double>(z) / Chunk::sz_z)),
  };
}

template <typename Func, typename... Args>
auto Region::get_data(int x, int y, int z, Func func, Args&&... args) {
  auto location = location_from_global_coords(x, y, z);
  auto& chunk = chunks_.at(location);
  return func(
    chunk,
    ((x % Chunk::sz_x) + Chunk::sz_x) % Chunk::sz_x,
    ((y % Chunk::sz_y) + Chunk::sz_y) % Chunk::sz_y,
    ((z % Chunk::sz_z) + Chunk::sz_z) % Chunk::sz_z,
    std::forward<Args>(args)...);
}

Voxel Region::get_voxel(int x, int y, int z) {
  return get_data(x, y, z, [](Chunk& chunk, int x, int y, int z) {
    return chunk.get_voxel(x, y, z);
  });
}

unsigned char Region::get_lighting(int x, int y, int z) {
  return get_data(x, y, z, [](Chunk& chunk, int x, int y, int z) {
    return chunk.get_lighting(x, y, z);
  });
}

void Region::set_voxel(int x, int y, int z, Voxel voxel) {
  get_data(x, y, z, [voxel](Chunk& chunk, int x, int y, int z) {
    chunk.set_voxel(x, y, z, voxel);
  });
}

void Region::set_lighting(int x, int y, int z, unsigned char lighting) {
  get_data(x, y, z, [lighting](Chunk& chunk, int x, int y, int z) {
    chunk.set_lighting(x, y, z, lighting);
  });
}

Player& Region::get_player() {
  return player_;
}

Section& Region::get_section(const Location2D loc) {
  return sections_.at(loc);
}

std::vector<Int3D> Region::raycast(Camera& camera) {
  auto& pos = camera.get_position();
  std::vector<Int3D> visited_voxels;

  // This id of the first/current voxel hit by the ray.
  // Using floor (round down) is actually very important,
  // the implicit int-casting will round up for negative numbers.
  Int3D current_voxel{
    static_cast<int>(std::floor(pos[0])),
    static_cast<int>(std::floor(pos[1])),
    static_cast<int>(std::floor(pos[2]))};

  // Compute normalized ray direction.
  auto& ray = camera.get_front();

  // In which direction the voxel ids are incremented.
  double stepX = (ray[0] >= 0) ? 1 : -1; // correct
  double stepY = (ray[1] >= 0) ? 1 : -1; // correct
  double stepZ = (ray[2] >= 0) ? 1 : -1; // correct

  // Distance along the ray to the next voxel border from the current position (tMaxX, tMaxY, tMaxZ).
  double next_voxel_boundary_x = (current_voxel[0] + stepX);
  double next_voxel_boundary_y = (current_voxel[1] + stepY);
  double next_voxel_boundary_z = (current_voxel[2] + stepZ);

  if (stepX < 0) {
    next_voxel_boundary_x += 1;
  }
  if (stepY < 0) {
    next_voxel_boundary_y += 1;
  }
  if (stepZ < 0) {
    next_voxel_boundary_z += 1;
  }
  // tMaxX, tMaxY, tMaxZ -- distance until next intersection with voxel-border
  // the value of t at which the ray crosses the first vertical voxel boundary
  double tMaxX = (ray[0] != 0) ? (next_voxel_boundary_x - pos[0]) / ray[0] : DBL_MAX; //
  double tMaxY = (ray[1] != 0) ? (next_voxel_boundary_y - pos[1]) / ray[1] : DBL_MAX; //
  double tMaxZ = (ray[2] != 0) ? (next_voxel_boundary_z - pos[2]) / ray[2] : DBL_MAX; //

  // tDeltaX, tDeltaY, tDeltaZ --
  // how far along the ray we must move for the horizontal component to equal the width of a voxel
  // the direction in which we traverse the grid
  // can only be FLT_MAX if we never go in that direction
  double tDeltaX = (ray[0] != 0) ? 1 / ray[0] * stepX : DBL_MAX;
  double tDeltaY = (ray[1] != 0) ? 1 / ray[1] * stepY : DBL_MAX;
  double tDeltaZ = (ray[2] != 0) ? 1 / ray[2] * stepZ : DBL_MAX;

  Int3D diff{0, 0, 0};
  bool neg_ray = false;
  if (ray[0] < 0) {
    diff[0]--;
    neg_ray = true;
  }
  if (ray[1] < 0) {
    diff[1]--;
    neg_ray = true;
  }
  if (ray[2] < 0) {
    diff[2]--;
    neg_ray = true;
  }
  visited_voxels.push_back(current_voxel);
  /*   if (neg_ray) {
      current_voxel += diff;
      visited_voxels.push_back(current_voxel);
    } */

  int limit = 50;
  int i = 0;
  while (i++ < limit) {
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

int Region::find_obstructing_height(Int3D root) const {
  int height = INT_MIN;
  auto loc = location_from_global_coords(root[0], root[1], root[2]);
  auto local = Chunk::to_local(root);
  int x = local[0];
  int y = local[1];
  int z = local[2];
  bool found = false;
  while (chunks_.contains(loc)) {
    auto& chunk = chunks_.at(loc);
    while (y-- > 0) {
      if (chunk.get_voxel(x, y, z) > Voxel::PARTIAL_OPAQUE_LOWER) {
        height = y + loc[1] * Chunk::sz_y;
        found = true;
        break;
      }
    }
    if (found)
      break;
    --loc[1];
    y = Chunk::sz_y - 1;
  }
  return height;
}

void Region::raycast_place(Camera& camera, Voxel voxel) {
  auto visited = raycast(camera);
  std::unordered_set<Location, LocationHash> dirty;
  for (int i = 0; i < visited.size(); ++i) {
    auto coord = visited[i];
    auto loc = location_from_global_coords(coord[0], coord[1], coord[2]);
    if (chunks_.contains(loc)) {
      auto& chunk = chunks_.at(loc);
      auto local = Chunk::to_local(coord);
      auto voxel_at_position = chunk.get_voxel(local[0], local[1], local[2]);
      if (voxel_at_position != Voxel::empty) {
        if (i == 0)
          return;

        auto coord = visited[i - 1];
        auto loc = location_from_global_coords(coord[0], coord[1], coord[2]);
        if (chunks_.contains(loc) && adjacents_missing_[loc] == 0) {
          auto& chunk = chunks_.at(loc);
          auto local = Chunk::to_local(coord);
          chunk.set_voxel(local[0], local[1], local[2], voxel);
          auto& section = sections_.at(Location2D{loc[0], loc[2]});
          if (section.get_subsection_obstructing_height(local[0], local[2]) < coord[1]) {
            section.set_subsection_obstructing_height(local[0], local[2], coord[1]);
          }

          chunks_.at(loc).compute_lighting(section);
          compute_global_lighting(loc);
          diffs_.emplace_back(Diff{loc, Diff::creation});

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
        }

        break;
      }
    }
  }
  for (auto& loc : dirty) {
    if (chunks_sent_.contains(loc) && adjacents_missing_[loc] == 0) {
      diffs_.emplace_back(Diff{loc, Diff::creation});
    }
  }
}

void Region::raycast_remove(Camera& camera) {
  auto visited = raycast(camera);
  std::unordered_set<Location, LocationHash> dirty;
  for (int i = 0; i < visited.size(); ++i) {
    auto v = visited[i];
    auto loc = location_from_global_coords(v[0], v[1], v[2]);
    if (chunks_.contains(loc) && adjacents_missing_[loc] == 0) {
      auto& chunk = chunks_.at(loc);
      auto local = Chunk::to_local(v);
      auto voxel = chunk.get_voxel(local[0], local[1], local[2]);
      if (voxel != Voxel::empty) {
        chunk.set_voxel(local[0], local[1], local[2], Voxel::empty);
        auto& section = sections_.at(Location2D{loc[0], loc[2]});
        if (section.get_subsection_obstructing_height(local[0], local[2]) == v[1]) {
          int height = find_obstructing_height(v);
          section.set_subsection_obstructing_height(local[0], local[2], height);
        }

        chunks_.at(loc).compute_lighting(section);
        compute_global_lighting(loc);
        diffs_.emplace_back(Diff{loc, Diff::creation});

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

        break;
      }
    }
  }

  for (auto& loc : dirty) {
    if (chunks_sent_.contains(loc) && adjacents_missing_[loc] == 0) {
      diffs_.emplace_back(Diff{loc, Diff::creation});
    }
  }
}