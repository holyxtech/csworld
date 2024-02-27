#include "common.h"
#include <iomanip>
#include <iostream>

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
    double lat = degrees + minutes / 60.0 + seconds / 3600.0;
    iss.clear();
    iss.str(lng_dms);
    iss >> degrees >> degreeSymbol >> minutes >> minuteSymbol >> seconds;
    double lng = degrees + minutes / 60.0 + seconds / 3600.0;
    std::cout<<lat<<","<<lng<<std::endl;
    pos[0] = (lng * equator_circumference) / (360);
    pos[2] = (lat * polar_circumference) / (180);
    std::cout<<pos[0]<<","<<pos[2]<<std::endl;
    return pos;
  }

} // namespace Common