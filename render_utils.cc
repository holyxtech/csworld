#include "render_utils.h"
#include "options.h"

namespace {
  std::string read_sfile(std::string path) {
    std::ifstream fs(path, std::ios::in);

    if (!fs.is_open()) {
      std::cerr << "Could not read file " << path << ". File does not exist." << std::endl;
      return "";
    }
    return std::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
  }

} // namespace

namespace RenderUtils {
  void preload_include(const std::string& path, const std::string& name) {
    auto source = read_sfile(path);
    glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, name.c_str(), -1, source.c_str());
  }

  void create_shader(GLuint* shader, const std::string& vertex_shader_path, const std::string& fragment_shader_path) {
    std::array<const char*, 1> search_dirs = {"/"};
    auto vertex_shader_source = read_sfile(vertex_shader_path);
    auto fragment_shader_source = read_sfile(fragment_shader_path);
    const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    auto* vertex_shader_source_cstr = vertex_shader_source.c_str();
    glShaderSource(vertex_shader, 1, &vertex_shader_source_cstr, nullptr);
    glCompileShaderIncludeARB(vertex_shader, search_dirs.size(), search_dirs.data(), nullptr);
    GLint success;
    GLchar info[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vertex_shader, 512, nullptr, info);
      std::cerr << "Failed to compile vertex shader: " << info << std::endl;
      throw std::runtime_error("Failed to initialize Renderer");
    }
    const auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    auto* fragment_shader_source_cstr = fragment_shader_source.c_str();
    glShaderSource(fragment_shader, 1, &fragment_shader_source_cstr, nullptr);
    glCompileShaderIncludeARB(fragment_shader, search_dirs.size(), search_dirs.data(), nullptr);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fragment_shader, 512, nullptr, info);
      std::cerr << "Failed to compile fragment shader: " << info << std::endl;
      throw std::runtime_error("Failed to initialize Renderer");
    }
    *shader = glCreateProgram();
    glAttachShader(*shader, vertex_shader);
    glAttachShader(*shader, fragment_shader);
    glLinkProgram(*shader);
    glGetProgramiv(*shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(*shader, 512, nullptr, info);
      std::cerr << "Failed to link shader program: " << info << std::endl;
      throw std::runtime_error("Failed to initialize Renderer");
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  }


} // namespace RenderUtils
