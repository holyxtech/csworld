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

  static std::pair<int, int> lat_lng_to_web_mercator(double latitude, double longitude, int zoom);
  static void calculateBoundingBox(int xtile, int ytile, int zoom, double& lng_deg, double& lat_deg);
  static std::pair<int, int> pixel_of_coord(int x, int y, int z, double lng, double lat);

  HTTPClient http_client_;
  static constexpr int zoom_level = 15;
  std::string elevation_data_url_prefix_ = "https://s3.amazonaws.com/elevation-tiles-prod/terrarium/" + std::to_string(zoom_level) + "/";
  std::unordered_map<std::pair<int, int>, Image, hash_pair> elevation_images_;

  static constexpr int equator_circumference = 40075000;
  static constexpr int polar_circumference = 40008000;
};

#endif