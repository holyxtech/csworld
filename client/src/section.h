#ifndef SECTION_H
#define SECTION_H

#include <unordered_map>
#include <vector>
#include "update_generated.h"
#include "common.h"
#include "types.h"

class Section {
public:
  static constexpr int sz_x = common::chunk_sz_x;
  static constexpr int sz_z = common::chunk_sz_z;
  static constexpr int sz = common::chunk_sz_x * common::chunk_sz_z;

  Section(const fbs_update::Section* section);
  const Location2D& get_location() const;
  int get_elevation() const;
  const std::vector<common::LandCover>& get_landcover() const;
  common::LandCover get_landcover(int x, int z) const;
  void set_elevation(int elevation);
  void compute_subsection_elevations(std::unordered_map<Location2D, Section, Location2DHash>& sections);
  bool has_subsection_elevations() const;
  const std::vector<int>& get_subsection_elevations() const;
  int get_subsection_elevation(int x, int z) const;
  void insert_into_features(int x, int y, int z, Voxel voxel);
  void set_features_loaded(bool loaded);
  bool is_features_loaded() const;
  std::vector<std::pair<int,Voxel>>& get_features(const Location& location);

private:
  Location2D location_;
  int elevation_;
  std::vector<common::LandCover> landcover_;

  std::vector<int> subsection_elevations_;
  bool computed_subsection_elevations_ = false;

  std::unordered_map<Location, std::vector<std::pair<int, Voxel>>, LocationHash> features_;
  bool features_loaded_ = false;
};

#endif
