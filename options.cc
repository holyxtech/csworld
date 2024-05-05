#include "options.h"

#include <iostream>


Options * Options::instance(int argc, char * argv[])
{
    if (Options::options_ != nullptr) return Options::options_;

    Options::options_ = new Options(argc, argv);

    return Options::options_;
}

Options::Options(int argc, char * argv[]) {
  if (argc <= 1) return;

  std::filesystem::path tmpPath = argv[APP_DIR];

  if (std::filesystem::exists(tmpPath) && std::filesystem::is_directory(tmpPath)) {
      this->dir = tmpPath;
  }
}

std::string Options::getShaderPath(const std::string& name)
{
  return this->getPath(name, SHADERS_DIR);
}

std::string Options::getImagePath(const std::string& name)
{
  return this->getPath(name, IMAGES_DIR);
}

std::string Options::getPath(const std::string& name, const std::string &type) {
  std::filesystem::path tmpPath = ((this->dir.has_value() ? this->dir.value() : std::filesystem::current_path()) / type);

  if (name.empty()) return tmpPath.string();

  return (tmpPath / name).string();
}

bool Options::hasValidShaderPath()
{
    std::filesystem::path actualDir = this->dir.has_value() ? this->dir.value() : std::filesystem::current_path();

    return std::filesystem::exists(actualDir / SHADERS_DIR) && std::filesystem::is_directory(actualDir / SHADERS_DIR);
}


Options::~Options() {}

Options * Options::options_ = nullptr;


