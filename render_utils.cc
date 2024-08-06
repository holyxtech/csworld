#include "render_utils.h"
#include <string>
#include "options.h"
#include "render_utils.h"
#include "stb_image.h"
#include "voxel.h"

namespace {
  std::array<const char*, 1> search_dirs = {"/"};
  bool shaders_included = false;
  std::string read_sfile(std::string path) {
    std::ifstream fs(path, std::ios::in);

    if (!fs.is_open()) {
      std::cerr << "Could not read file " << path << ". File does not exist." << std::endl;
      return "";
    }
    return std::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
  }
  void include_shader(const std::string& path, const std::string& name) {
    auto source = read_sfile(path);
    glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, name.c_str(), -1, source.c_str());
  }
  void include_all() {
    include_shader(Options::instance()->getShaderPath("common.glsl"), "/common.glsl");
    include_shader(Options::instance()->getShaderPath("shadow.glsl"), "/shadow.glsl");
    include_shader(Options::instance()->getShaderPath("ssr.glsl"), "/ssr.glsl");
    shaders_included = true;
  }
} // namespace

namespace RenderUtils {

  GLuint compile_shader(const std::string& path, GLenum shader_type) {
    auto shader_source = read_sfile(path);
    const auto shader = glCreateShader(shader_type);
    auto* c_str = shader_source.c_str();
    glShaderSource(shader, 1, &c_str, nullptr);
    glCompileShaderIncludeARB(shader, search_dirs.size(), search_dirs.data(), nullptr);
    GLint success;
    GLchar info[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 512, nullptr, info);
      std::cerr << "Failed to compile shader at " << path << ": " << info << std::endl;
      throw std::runtime_error("Failed to initialize Renderer");
    }
    return shader;
  }

  GLuint create_shader(const std::string& vertex_shader_path, const std::string& geometry_shader_path, const std::string& fragment_shader_path) {
    if (!shaders_included)
      include_all();
    GLuint vertex_shader = compile_shader(vertex_shader_path, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_shader_path, GL_FRAGMENT_SHADER);
    GLuint geometry_shader = compile_shader(geometry_shader_path, GL_GEOMETRY_SHADER);
    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glAttachShader(shader, geometry_shader);
    glLinkProgram(shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteShader(geometry_shader);
    return shader;
  }

  GLuint create_shader(const std::string& vertex_shader_path, const std::string& fragment_shader_path) {
    if (!shaders_included)
      include_all();
    GLuint vertex_shader = compile_shader(vertex_shader_path, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_shader_path, GL_FRAGMENT_SHADER);
    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return shader;
  }

  std::vector<glm::vec3> get_frustum_corners_world_space(const glm::mat4& proj, const glm::mat4& view) {
    const auto inv = glm::inverse(proj * view);
    std::vector<glm::vec3> corners;
    for (int x = 0; x < 2; ++x) {
      for (int y = 0; y < 2; ++y) {
        for (int z = 0; z < 2; ++z) {
          const glm::vec4 pt =
            inv *
            glm::vec4(
              2.0f * x - 1.0f,
              2.0f * y - 1.0f,
              2.0f * z - 1.0f,
              1.0f);
          corners.push_back(pt / pt.w);
        }
      }
    }
    return corners;
  }

} // namespace RenderUtils
