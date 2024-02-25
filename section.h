#ifndef SECTION_H
#define SECTION_H

#include <unordered_map>
#include <vector>
#include "chunk.h"
#include "types.h"

class Section {
public:
  Section(Location2D location);
  const Location2D& get_location() const;
  int get_elevation() const;
  void set_elevation(int elevation);
  void compute_subsection_elevations(std::unordered_map<Location2D, Section, Location2DHash>& sections);
  bool has_subsection_elevations() const;
  const std::vector<int>& get_subsection_elevations() const;
  int get_subsection_elevation(int x, int z) const;

private:
  Location2D location_;
  int elevation_;

  std::vector<int> subsection_elevations_;
  bool computed_subsection_elevations_ = false;

  static constexpr int sz_x = Chunk::sz_x;
  static constexpr int sz_z = Chunk::sz_y;
  static constexpr int sz = sz_x * sz_z;
};

#endif