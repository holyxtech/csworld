#include "section.h"
#include "chunk.h"
#include "region.h"

Section::Section(const fbs_update::Section* section) {
  auto loc = section->location();
  location_ = Location2D{loc->x(), loc->y()};
  elevation_ = section->elevation();
  for (int i = 0; i < Common::landcover_tiles_per_sector; ++i)
    landcover_[i] = static_cast<Common::LandCover>(section->landcover()->Get(i));
}

const Location2D& Section::get_location() const {
  return location_;
}

int Section::get_elevation() const {
  return elevation_;
}

const std::array<Common::LandCover, Common::landcover_tiles_per_sector>& Section::get_landcover() const {
  return landcover_;
}

void Section::set_elevation(int elevation) {
  elevation_ = elevation;
}

// Assume only called when all neighbouring sections are present
void Section::compute_subsection_elevations(std::unordered_map<Location2D, Section, Location2DHash>& sections) {
  // subsection_elevations_.assign(sz, elevation_);

  Location2D loc{location_[0] - 1, location_[1] + 1};
  int e1 = sections.at(loc).elevation_;
  loc[0]++;
  int e2 = sections.at(loc).elevation_;
  loc[0]++;
  int e3 = sections.at(loc).elevation_;
  loc[1]--;
  int e4 = sections.at(loc).elevation_;
  loc[1]--;
  int e5 = sections.at(loc).elevation_;
  loc[0]--;
  int e6 = sections.at(loc).elevation_;
  loc[0]--;
  int e7 = sections.at(loc).elevation_;
  loc[1]++;
  int e8 = sections.at(loc).elevation_;

  // e1 | e2 | e3
  // e8 | e  | e4
  // e7 | e6 | e5

  float a1 = (e8 + e1 + e2 + elevation_) / 4.0;
  float a2 = (e2 + e3 + e4 + elevation_) / 4.0;
  float a3 = (e4 + e5 + e6 + elevation_) / 4.0;
  float a4 = (e6 + e7 + e8 + elevation_) / 4.0;

  float vAAu = ((e2 + elevation_) - (e1 + e8));
  float vAAv = ((e1 + e2) - (e8 + elevation_));
  float vABu = ((e3 + e4) - (e2 + elevation_));
  float vABv = ((e2 + e3) - (elevation_ + e4));
  float vBAu = ((elevation_ + e6) - (e8 + e7));
  float vBAv = ((e8 + elevation_) - (e7 + e6));
  float vBBu = ((e4 + e5) - (elevation_ + e6));
  float vBBv = ((elevation_ + e4) - (e6 + e5));

  for (float z = 0; z < sz_z; ++z) {
    for (float x = 0; x < sz_x; ++x) {
      float u = (x + 0.5) / sz_x;
      float v = (z + 0.5) / sz_z;
      float u_fade = u * u * (3 - 2 * u);
      float v_fade = v * v * (3 - 2 * v);

      u /= 2;
      v /= 2;
      float iu = u - 0.5;
      float iv = v - 0.5;

      float n_x0 = (1 - u_fade) * (a4 + vBAu * u + vBAv * v) + u_fade * (a3 + vBBu * iu + vBBv * v);
      float n_x1 = (1 - u_fade) * (a1 + vAAu * u + vAAv * iv) + u_fade * (a2 + vABu * iu + vABv * iv);

      int n_xy = (1 - v_fade) * n_x0 + v_fade * n_x1;
      subsection_elevations_[x + sz_x * z] = n_xy;
    }
  }

  computed_subsection_elevations_ = true;
}

bool Section::has_subsection_elevations() const {
  return computed_subsection_elevations_;
}

const std::array<int, Section::sz>& Section::get_subsection_elevations() const {
  return subsection_elevations_;
}

int Section::get_subsection_elevation(int x, int z) const {
  return subsection_elevations_[x + sz_x * z];
}

Common::LandCover Section::get_landcover(int x, int z) const {
  int col = x * Common::landcover_cols_per_sector / Section::sz_x;
  int row = z * Common::landcover_rows_per_sector / Section::sz_z;
  return landcover_[col + row * Common::landcover_cols_per_sector];
}

void Section::set_subsection_obstructing_heights_from_elevations() {
  for (size_t i = 0; i < sz; ++i) {
    subsection_obstructing_heights_[i] = subsection_elevations_[i];
  }
}

int Section::get_subsection_obstructing_height(int x, int z) {
  return subsection_obstructing_heights_[x + sz_x * z];
}

void Section::set_subsection_obstructing_height(int x, int z, int height) {
  subsection_obstructing_heights_[x + sz_x * z] = height;
}

void Section::insert_into_features(int x, int y, int z, Voxel voxel) {
  auto location = Region::location_from_global_coords(x, y, z);
  auto& location_features = features_[location];
  int idx = Chunk::get_index(Chunk::to_local(Int3D{x, y, z}));
  location_features.emplace_back(std::make_pair(idx, voxel));
}

void Section::set_features_loaded(bool loaded) {
  features_loaded_ = loaded;
}

bool Section::is_features_loaded() const {
  return features_loaded_;
}

std::vector<std::pair<int,Voxel>>& Section::get_features(const Location& location) {
  return features_[location];
}