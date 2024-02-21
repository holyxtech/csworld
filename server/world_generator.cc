#include "world_generator.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::string decimalToDMS(double decimalValue) {
  bool isNegative = decimalValue < 0;
  decimalValue = std::abs(decimalValue);
  int degrees = static_cast<int>(decimalValue);
  double remainingMinutes = (decimalValue - degrees) * 60.0;
  int minutes = static_cast<int>(remainingMinutes);
  double seconds = (remainingMinutes - minutes) * 60.0;
  std::stringstream dmsStream;
  dmsStream << std::fixed << std::setprecision(6);
  dmsStream << (isNegative ? "-" : "") << degrees << "°" << minutes << "'" << seconds << "\"";
  return dmsStream.str();
}

Section WorldGenerator::get_section(Location2D loc) {
  Section section;
  // 1. convert chunk (x,z) coords to lat/long (equirectangular projection)
  double lng = 360.0 * (loc[0] * Chunk::sz_x) / equator_circumference;
  double lat = 180.0 * (loc[1] * Chunk::sz_z) / polar_circumference;
  //   2. convert lat/long to tile
  auto tile = lat_lng_to_web_mercator(lat, lng, zoom_level);

  // 3. pull tile
  unsigned char* image;
  int width, height, channels;
  auto it = elevation_images_.find(tile);
  if (it != elevation_images_.end()) {
    image = it->second.data;
    width = it->second.width;
    channels = it->second.channels;
  } else {
    // Look for it locally, if it's not there download it
    std::string image_binary;
    std::string output_filename = std::to_string(tile.first) + "-" + std::to_string(tile.second) + ".png";
    std::string output_path = "./images/" + output_filename;
    std::ifstream file;
    file.open(output_path, std::ios::binary);
    if (file) {
      image_binary.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    } else {
      std::string filename = output_filename;
      std::replace(filename.begin(), filename.end(), '-', '/');
      std::string url = elevation_data_url_prefix_ + filename;
      image_binary = http_client_.make_request(url);
    }
    // Load it and save it if necessary
    image = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(image_binary.data()), image_binary.size(),
                                  &width, &height, &channels, 0);
    elevation_images_[tile] = {image, width, height, channels};
    if (!file)
      stbi_write_png(output_path.c_str(), width, height, channels, image, width * channels);
  }
  // 4. find chunk location within tile
  auto [x, y] = pixel_of_coord(tile.first, tile.second, zoom_level, lng, lat);
  //  5. inspect that pixel
  int pixel_index = (y * width + x) * channels;
  int red = static_cast<int>(image[pixel_index]);
  int green = static_cast<int>(image[pixel_index + 1]);
  int blue = static_cast<int>(image[pixel_index + 2]);
  section.elevation = (red * 256 + green + blue / 256) - 32768;

  return section;
}

WorldGenerator::~WorldGenerator() {
  for (auto& pair : elevation_images_) {
    auto* image = pair.second.data;
    stbi_image_free(image);
  }
}

std::pair<int, int> WorldGenerator::lat_lng_to_web_mercator(double latitude, double longitude, int zoom) {
  double longitude_in_radians = longitude * M_PI / 180;
  double latitude_in_radians = latitude * M_PI / 180;

  double x = pow(2, zoom) * (M_PI + longitude_in_radians) / (2 * M_PI);
  double y = pow(2, zoom) * (M_PI - log(tan((M_PI / 4) + (latitude_in_radians / 2)))) / (2 * M_PI);
  int xTile = static_cast<int>(x);
  int yTile = static_cast<int>(y);

  return std::make_pair(xTile, yTile);
}

void WorldGenerator::calculateBoundingBox(int xtile, int ytile, int zoom, double& lng_deg, double& lat_deg) {
  double n = std::pow(2.0, zoom);
  lng_deg = xtile / n * 360.0 - 180.0;
  double lat_rad = std::atan(std::sinh(M_PI * (1 - 2.0 * ytile / n)));
  lat_deg = lat_rad * 180.0 / M_PI;
}

std::pair<int, int> WorldGenerator::pixel_of_coord(int x, int y, int z, double lng, double lat) {
  int tile_max_x = 255;
  int tile_max_y = 255;

  double lng_deg_min, lat_deg_min;
  double lng_deg_max, lat_deg_max;
  calculateBoundingBox(x, y, z, lng_deg_min, lat_deg_max);
  calculateBoundingBox(x + 1, y + 1, z, lng_deg_max, lat_deg_min);

  int pixel_x = tile_max_x * (std::abs(lng - lng_deg_min)) / (std::abs(lng_deg_min - lng_deg_max));
  int pixel_y = tile_max_y * (std::abs(lat - lat_deg_max)) / (std::abs(lat_deg_min - lat_deg_max));

  return std::make_pair(pixel_x, pixel_y);
}
