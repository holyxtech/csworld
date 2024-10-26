#ifndef OPTIONS_H
#define OPTIONS_H

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

class Options final {
public:
  static Options* instance(int argc = 0, char* argv[] = nullptr);

  Options(const Options& other) = delete;
  Options* operator=(const Options* other) = delete;

  std::string get_shader_path(const std::string& name);
  std::string get_image_path(const std::string& name);
  std::string get_font_path(const std::string& name);
  std::string get_ui_path(const std::string& name);
  static int window_width;
  static int window_height;

private:
  static constexpr const char* shaders_dir = "shaders";
  static constexpr const char* images_dir = "assets/images";
  static constexpr const char* fonts_dir = "assets/fonts";
  static constexpr const char* ui_dir = "assets/ui";

  std::string get_path(const std::string& name, const std::string& type);
  Options(int argc = 0, char* argv[] = nullptr);

  std::optional<std::filesystem::path> dir;
};

#endif
