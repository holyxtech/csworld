#ifndef OPTIONS_H
#define OPTIONS_H

#define SHADERS_DIR "shaders"
#define IMAGES_DIR "assets/images"
#define APP_DIR 1

#include <filesystem>
#include <memory>
#include <optional>

class Options final {
public:
  static std::shared_ptr<Options> instance(int argc = 0, char* argv[] = nullptr);

  ~Options();
  Options(const Options& other) = delete;
  Options* operator=(const Options* other) = delete;

  std::string getShaderPath(const std::string& name);
  std::string getImagePath(const std::string& name);
  bool hasValidShaderPath();

private:
  static std::shared_ptr<Options> options_;

  std::string getPath(const std::string& name, const std::string& type);
  Options(int argc = 0, char* argv[] = nullptr);

  std::optional<std::filesystem::path> dir;
};

#endif
