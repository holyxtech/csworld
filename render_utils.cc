#include "render_utils.h"

namespace {
  std::string read_shader_file(std::string path) {
    std::ifstream fs(path, std::ios::in);

    if (!fs.is_open()) {
      std::cerr << "Could not read file " << path << ". File does not exist." << std::endl;
      return "";
    }
    return std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
  }

} // namespace

namespace RenderUtils {
  void create_shader(GLuint* shader, std::string vertex_shader_path, std::string fragment_shader_path) {
    auto vertex_shader_source = read_shader_file(vertex_shader_path);
    auto fragment_shader_source = read_shader_file(fragment_shader_path);
    const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    auto* vertex_shader_source_cstr = vertex_shader_source.c_str();
    glShaderSource(vertex_shader, 1, &vertex_shader_source_cstr, nullptr);
    glCompileShader(vertex_shader);
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
    glCompileShader(fragment_shader);
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