#include "options.h"

#include <iostream>

int Options::window_width = 2560;
int Options::window_height = 1440;

Options* Options::instance(int argc, char* argv[]) {
  static Options* instance = new Options(argc, argv);
  return instance;
}

Options::Options(int argc, char* argv[]) {
  if (argc < 2)
    throw std::invalid_argument("No path provided to assets.");

  std::filesystem::path dir = argv[1];
  if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir))
    this->dir = dir;
  else
    throw std::invalid_argument("Path provided is not an existing directory.");
}

std::string Options::get_shader_path(const std::string& name) {
  return get_path(name, shaders_dir);
}

std::string Options::get_image_path(const std::string& name) {
  return get_path(name, images_dir);
}

std::string Options::get_font_path(const std::string& name) {
  return get_path(name, fonts_dir);
}

std::string Options::get_path(const std::string& name, const std::string& type) {
  std::filesystem::path dir = ((this->dir.has_value() ? this->dir.value() : std::filesystem::current_path()) / type);
  if (name.empty())
    return dir.string();
  return (dir / name).string();
}