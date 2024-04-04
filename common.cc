#include "common.h"
#include <iomanip>
#include <iostream>
#include <random>

namespace {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> uniform_probability(0.0f, 1.0f);

} // namespace

namespace Common {

  double dms_to_decimal(const std::string& dms) {
    return 0;
  }

  std::string decimal_to_dms(double val) {
    bool is_negative = val < 0;
    val = std::abs(val);
    int degrees = static_cast<int>(val);
    double mins = (val - degrees) * 60.0;
    int minutes = static_cast<int>(mins);
    double seconds = (mins - minutes) * 60.0;
    std::stringstream stream;
    stream << std::fixed << std::setprecision(6);
    stream << (is_negative ? "-" : "") << degrees << "°" << minutes << "'" << seconds << "\"";
    return stream.str();
  }

  std::array<double, 3> lat_lng_to_world_pos(const std::string& lat_dms, const std::string& lng_dms) {
    std::array<double, 3> pos;
    double degrees, minutes, seconds;
    char degreeSymbol, minuteSymbol;
    std::istringstream iss(lat_dms);
    iss >> degrees >> degreeSymbol >> minutes >> minuteSymbol >> seconds;
    double lat;
    if (degrees < 0)
      lat = degrees - minutes / 60.0 - seconds / 3600.0;
    else
      lat = degrees + minutes / 60.0 + seconds / 3600.0;
    iss.clear();
    iss.str(lng_dms);
    iss >> degrees >> degreeSymbol >> minutes >> minuteSymbol >> seconds;
    double lng;
    if (degrees < 0)
      lng = degrees - minutes / 60.0 - seconds / 3600.0;
    else
      lng = degrees + minutes / 60.0 + seconds / 3600.0;
    pos[0] = (lng * equator_circumference) / (360);
    pos[2] = (lat * (polar_circumference / 2)) / (180);
    return pos;
  }

  float random_probability() {
    return uniform_probability(gen);
  }

  float random_float(float low, float high) {
    std::uniform_real_distribution<float> dist(low, high);
    return dist(gen);
  };

  int random_int(int low, int high) {
    std::uniform_int_distribution<int> dist(low, high);
    return dist(gen);
  }

  // Stateless and repeatable function that returns a
  // pseduo-random number in the range [0, 0xFFFFFFFF].
  std::uint32_t Hash(const std::uint32_t seed) {
    // So that we can use unsigned int literals, e.g. 42u.
    static_assert(sizeof(unsigned int) == sizeof(std::uint32_t),
                  "integer size mismatch");

    auto i = std::uint32_t{(seed ^ 12345391U) * 2654435769U}; // NOLINT
    i ^= (i << 6U) ^ (i >> 26U);                              // NOLINT
    i *= 2654435769U;                                         // NOLINT
    i += (i << 5U) ^ (i >> 12U);                              // NOLINT
    return i;
  }

  // Returns a pseduo-random number in the range [0, 0xFFFFFFFF].
  // Note that seed is incremented for each invokation.
  std::uint32_t Rand(std::uint32_t const seed) {
    // Not worrying about seed "overflow" since it is unsigned.
    return Hash(seed);
  }

  // Returns a pseduo-random number in the range [0, 1].
  float NormRand(std::uint32_t const seed) {
    return (1 / static_cast<float>(std::numeric_limits<std::uint32_t>::max())) *
           static_cast<float>(Rand(seed));
  }

  // Returns a pseduo-random number in the range [offset, offset + range].
  // Assumes range > 0.
  float RangeRand(const float offset, const float range,
                  std::uint32_t const seed) {
    return offset + range * NormRand(seed);
  }

} // namespace Common