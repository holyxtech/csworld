#include "render_utils.h"
#include <algorithm>
#include <cctype>
#include <string>
#include "options.h"
#include "render_utils.h"
#include "stb_image.h"
#include "voxel.h"

namespace {
  std::string read_shader_file(const std::string& path) {
    std::ifstream fs(path, std::ios::in);

    if (!fs.is_open()) {
      std::cerr << "Could not read file " << path << ". File does not exist." << std::endl;
      return "";
    }

    const std::string include_match = "#include ";
    std::stringstream file_content;

    std::string line;
    while (std::getline(fs, line)) {
      std::size_t include_found = line.find(include_match);
      if (include_found != std::string::npos) {
        // erase include prefix
        line.erase(line.begin(), line.begin() + include_found + include_match.length());
        // erase whitespace, <,> and "
        line.erase(
          std::remove_if(line.begin(), line.end(), [](const char c) {
            return std::isspace(c) || c == '<' || c == '>' || c == '"';
          }),
          line.end());
        // what's left is the include name
        const auto include_file_location = Options::instance()->get_shader_path(line);
        if (std::filesystem::exists(include_file_location)) {
          file_content << read_shader_file(include_file_location);
        }
      } else
        file_content << line << std::endl;
    }

    fs.close();

    return file_content.str();
  }
} // namespace

namespace RenderUtils {

  GLuint compile_shader(const std::string& name, GLenum shader_type) {
    auto path = Options::instance()->get_shader_path(name);
    auto shader_source = read_shader_file(path);
    const auto shader = glCreateShader(shader_type);

    auto* c_str = shader_source.c_str();
    glShaderSource(shader, 1, &c_str, nullptr);
    glCompileShader(shader);
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

  GLuint create_shader(const std::string& compute_shader_name) {
    GLuint compute_shader = compile_shader(compute_shader_name, GL_COMPUTE_SHADER);
    GLuint shader = glCreateProgram();

    glAttachShader(shader, compute_shader);

    glLinkProgram(shader);
    glDeleteShader(compute_shader);

    return shader;
  }

  GLuint create_shader(const std::string& vertex_shader_name, const std::string& geometry_shader_name, const std::string& fragment_shader_name) {
    GLuint vertex_shader = compile_shader(vertex_shader_name, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_shader_name, GL_FRAGMENT_SHADER);
    GLuint geometry_shader = compile_shader(geometry_shader_name, GL_GEOMETRY_SHADER);
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

  GLuint create_shader(const std::string& vertex_shader_name, const std::string& fragment_shader_name) {
    GLuint vertex_shader = compile_shader(vertex_shader_name, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_shader_name, GL_FRAGMENT_SHADER);
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
