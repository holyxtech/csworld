#ifndef COMMON_H
#define COMMON_H

#include <array>
#include <cstdint>
#include <string>

namespace Common {
  std::string decimal_to_dms(double val);
  double dms_to_decimal(const std::string& dms);
  std::array<double, 3> lat_lng_to_world_pos(const std::string& lat_dms, const std::string& lng_dms);

  std::uint32_t Hash(const std::uint32_t seed);
  std::uint32_t Rand(std::uint32_t const seed);
  float NormRand(std::uint32_t const seed);
  float RangeRand(const float offset, const float range, std::uint32_t const seed);

  constexpr int equator_circumference = 40075000;
  constexpr int polar_circumference = 40008000;
  constexpr int chunk_sz_uniform = 16;
  constexpr int chunk_sz_x = chunk_sz_uniform;
  constexpr int chunk_sz_y = chunk_sz_uniform;
  constexpr int chunk_sz_z = chunk_sz_uniform;
  constexpr int chunk_sz = chunk_sz_x * chunk_sz_y * chunk_sz_z;
  constexpr int landcover_rows_per_sector = 1;
  constexpr int landcover_cols_per_sector = 1;
  constexpr int landcover_tiles_per_sector = landcover_rows_per_sector * landcover_cols_per_sector;

  enum class LandCover : uint8_t {
    bare,
    water,
    trees,
    grass,
    shrubs,
    snow,
    wetland,
    mangroves,
    moss
  };

  float random_probability();
  float random_float(float low, float high);
  int random_int(int low, int high);

} // namespace Common

#endif