#ifndef SECTION_H
#define SECTION_H

#include <unordered_map>
#include <vector>
#include "./fbs/update_generated.h"
#include "common.h"
#include "types.h"

class Section {
public:
  static constexpr int sz_x = Common::chunk_sz_x;
  static constexpr int sz_z = Common::chunk_sz_z;
  static constexpr int sz = Common::chunk_sz_x * Common::chunk_sz_z;

  Section(const fbs_update::Section* section);
  const Location2D& get_location() const;
  int get_elevation() const;
  const std::array<Common::LandCover, Common::landcover_tiles_per_sector>& get_landcover() const;
  Common::LandCover get_landcover(int x, int z) const;
  void set_elevation(int elevation);
  void compute_subsection_elevations(std::unordered_map<Location2D, Section, Location2DHash>& sections);
  bool has_subsection_elevations() const;
  const std::array<int, sz>& get_subsection_elevations() const;
  int get_subsection_elevation(int x, int z) const;

private:
  Location2D location_;
  int elevation_;
  std::array<Common::LandCover, Common::landcover_tiles_per_sector> landcover_;
  // Common::LandCover landcover_;

  std::array<int, sz> subsection_elevations_;
  bool computed_subsection_elevations_ = false;
};

#endif