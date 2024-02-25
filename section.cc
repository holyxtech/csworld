#include "section.h"

Section::Section(Location2D location) {
  location_ = location;
  subsection_elevations_.reserve(sz);
}

const Location2D& Section::get_location() const {
  return location_;
}

int Section::get_elevation() const {
  return elevation_;
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

  for (float z = 0; z < sz_z; ++z) {
    for (float x = 0; x < sz_x; ++x) {
      float u = (x + 0.5) / sz_x;
      float v = (z + 0.5) / sz_z;
      float u_fade = u * u * (3 - 2 * u);
      float v_fade = v * v * (3 - 2 * v);

      float n_x0 = (1 - u_fade) * (a4) + u_fade * (a3);
      float n_x1 = (1 - u_fade) * (a1) + u_fade * (a2);

      int n_xy = (1 - v_fade) * n_x0 + v_fade * n_x1;
      subsection_elevations_.emplace_back(n_xy);
    }
  }

  computed_subsection_elevations_ = true;
}

bool Section::has_subsection_elevations() const {
  return computed_subsection_elevations_;
}

const std::vector<int>& Section::get_subsection_elevations() const {
  return subsection_elevations_;
}

int Section::get_subsection_elevation(int x, int z) const {
  return subsection_elevations_[x + sz_x * z];
}