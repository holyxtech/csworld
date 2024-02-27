#ifndef COMMON_H
#define COMMON_H

#include <array>
#include <string>

namespace Common {
  std::string decimal_to_dms(double val);
  double dms_to_decimal(const std::string& dms);
  std::array<double, 3> lat_lng_to_world_pos(const std::string& lat_dms, const std::string& lng_dms);

  constexpr int equator_circumference = 40075000;
  constexpr int polar_circumference = 40008000;
  constexpr int chunk_sz_uniform = 64;
  constexpr int chunk_sz_x = chunk_sz_uniform;
  constexpr int chunk_sz_y = chunk_sz_uniform;
  constexpr int chunk_sz_z = chunk_sz_uniform;
  constexpr int chunk_sz = chunk_sz_x * chunk_sz_y * chunk_sz_z;

  enum LandCover {
    barren,
    water
  };
} // namespace Common

#endif