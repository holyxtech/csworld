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

} // namespace Common