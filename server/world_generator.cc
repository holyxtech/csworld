#include "world_generator.h"
#include <fstream>
#include <functional>
#include <iomanip>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../common.h"
#include "../config.h" // Include the configured constants
#include "stb_image_write.h"

WorldGenerator::Image WorldGenerator::get_image(
  std::pair<int, int> tile,
  std::string dir,
  std::unordered_map<std::pair<int, int>, Image, hash_pair>& image_store,
  std::string url) {

  auto it = image_store.find(tile);
  if (it != image_store.end()) {
    return it->second;
  } else {
    // Look for it locally, if it's not there download it
    std::string image_binary;
    std::string output_filename = std::to_string(tile.first) + "-" + std::to_string(tile.second) + ".png";
    std::string output_path = dir + output_filename;
    std::ifstream file;
    file.open(output_path, std::ios::binary);
    if (file) {
      image_binary.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    } else {
      image_binary = http_client_.make_request(url);
    }
    // Load it and save it if necessary
    unsigned char* data;
    int width, height, channels;
    data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(image_binary.data()), image_binary.size(),
                                 &width, &height, &channels, 0);
    Image image = {data, width, height, channels};
    image_store[tile] = image;
    if (!file)
      stbi_write_png(output_path.c_str(), width, height, channels, data, width * channels);
    return image;
  }
}

Section WorldGenerator::get_section(Location2D loc) {
  Section section;

  double lng = 360.0 * (loc[0] * Chunk::sz_x) / Common::equator_circumference;
  double lat = 180.0 * (loc[1] * Chunk::sz_z) / (Common::polar_circumference / 2);
  auto tile = lat_lng_to_web_mercator(lat, lng, zoom_level);

  {
    auto image = get_image(
      tile, std::string(APPLICATION_DATA_DIR) + "/images/elevation/", elevation_images_,
      "https://s3.amazonaws.com/elevation-tiles-prod/terrarium/" + std::to_string(zoom_level) +
        "/" + std::to_string(tile.first) + "/" + std::to_string(tile.second) + ".png");

    auto [data, width, height, channels] = image;
    auto [x, y] = pixel_of_coord(tile.first, tile.second, zoom_level, lng, lat);
    int pixel_index = (y * width + x) * channels;
    int red = static_cast<int>(data[pixel_index]);
    int green = static_cast<int>(data[pixel_index + 1]);
    int blue = static_cast<int>(data[pixel_index + 2]);
    section.elevation = (red * 256 + green + blue / 256) - 32768;
  }
  {
    int num_rows = Common::landcover_rows_per_sector;
    int num_cols = Common::landcover_cols_per_sector;
    std::string img_url =
      "https://services.terrascope.be/wmts/v2?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0"
      "&LAYER=WORLDCOVER_2021_MAP&STYLE=default&FORMAT=image/jpeg&TILEMATRIXSET=EPSG%3A3857&TILEMATRIX=EPSG:3857:" +
      std::to_string(zoom_level) + "&TILECOL=" + std::to_string(tile.first) + "&TILEROW=" + std::to_string(tile.second);
    auto image = get_image(tile, std::string(APPLICATION_DATA_DIR) + "/images/landcover/", landcover_images_, img_url);

    for (int row = 0; row < num_rows; ++row) {
      for (int col = 0; col < num_cols; ++col) {
        auto loc_x = (loc[0] + col / static_cast<float>(num_cols)) * Chunk::sz_x;
        auto loc_z = (loc[1] + row / static_cast<float>(num_rows)) * Chunk::sz_z;
        double lng = 360.0 * loc_x / Common::equator_circumference;
        double lat = 180.0 * loc_z / (Common::polar_circumference / 2);
        auto [x, y] = pixel_of_coord(tile.first, tile.second, zoom_level, lng, lat);

        if (x < 0 || x > tile_max_x || y < 0 || y > tile_max_y) {
          tile = lat_lng_to_web_mercator(lat, lng, zoom_level);
          std::tie(x, y) = pixel_of_coord(tile.first, tile.second, zoom_level, lng, lat);
          image = get_image(tile, std::string(APPLICATION_DATA_DIR) + "/images/landcover/", landcover_images_, img_url);
        }

        auto [data, width, height, channels] = image;

        int pixel_index = (y * width + x) * channels;

        int red = static_cast<int>(data[pixel_index]);
        int green = static_cast<int>(data[pixel_index + 1]);
        int blue = static_cast<int>(data[pixel_index + 2]);
        int val = ((red << 16) | (green << 8) | blue);

        switch (val) {
        case 25800:
          section.landcover[row * num_cols + col] = Common::LandCover::water;
          break;
        case 25600:
          section.landcover[row * num_cols + col] = Common::LandCover::trees;
          break;
        case 16777036:
          section.landcover[row * num_cols + col] = Common::LandCover::grass;
          break;
        case 16759586:
          section.landcover[row * num_cols + col] = Common::LandCover::shrubs;
          break;
        case 11842740:
          section.landcover[row * num_cols + col] = Common::LandCover::bare;
          break;
        case 15790320:
          section.landcover[row * num_cols + col] = Common::LandCover::snow;
          break;
        case 38560:
          section.landcover[row * num_cols + col] = Common::LandCover::wetland;
          break;
        case 53109:
          section.landcover[row * num_cols + col] = Common::LandCover::mangroves;
          break;
        case 16443040:
          section.landcover[row * num_cols + col] = Common::LandCover::moss;
          break;
        case 15767295:
          section.landcover[row * num_cols + col] = Common::LandCover::grass;
          break;
        }
      }
    }
  }

  return section;
}

WorldGenerator::~WorldGenerator() {
  for (auto& pair : elevation_images_) {
    auto* image = pair.second.data;
    stbi_image_free(image);
  }
  for (auto& pair : landcover_images_) {
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

void WorldGenerator::calculate_bounding_box(int xtile, int ytile, int zoom, double& lng_deg, double& lat_deg) {
  double n = std::pow(2.0, zoom);
  lng_deg = xtile / n * 360.0 - 180.0;
  double lat_rad = std::atan(std::sinh(M_PI * (1 - 2.0 * ytile / n)));
  lat_deg = lat_rad * 180.0 / M_PI;
}

std::pair<int, int> WorldGenerator::pixel_of_coord(int x, int y, int z, double lng, double lat) {

  double lng_deg_min, lat_deg_min;
  double lng_deg_max, lat_deg_max;
  calculate_bounding_box(x, y, z, lng_deg_min, lat_deg_max);
  calculate_bounding_box(x + 1, y + 1, z, lng_deg_max, lat_deg_min);

  int pixel_x = tile_max_x * (std::abs(lng - lng_deg_min)) / (std::abs(lng_deg_min - lng_deg_max));
  int pixel_y = tile_max_y * (std::abs(lat - lat_deg_max)) / (std::abs(lat_deg_min - lat_deg_max));

  return std::make_pair(pixel_x, pixel_y);
}
