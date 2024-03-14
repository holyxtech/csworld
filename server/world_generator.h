#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <string>
#include <unordered_map>
#include "chunk.h"
#include "http_client.h"
#include "types.h"

class WorldGenerator {
public:
  // void fill_chunk(Chunk& chunk);
  Section get_section(Location2D loc);
  ~WorldGenerator();

private:
  struct Image {
    unsigned char* data;
    int width;
    int height;
    int channels;
  };

  static constexpr int tile_max_x = 255;
  static constexpr int tile_max_y = 255;

  Image get_image(
    std::pair<int, int> tile,
    std::string dir,
    std::unordered_map<std::pair<int, int>, Image, hash_pair>& image_store,
    std::string url);

  static std::pair<int, int> lat_lng_to_web_mercator(double latitude, double longitude, int zoom);
  static void calculate_bounding_box(int xtile, int ytile, int zoom, double& lng_deg, double& lat_deg);
  static std::pair<int, int> pixel_of_coord(int x, int y, int z, double lng, double lat);

  HTTPClient http_client_;
  static constexpr int zoom_level = 15;
  std::unordered_map<std::pair<int, int>, Image, hash_pair> elevation_images_;
  std::unordered_map<std::pair<int, int>, Image, hash_pair> landcover_images_;
};

#endif