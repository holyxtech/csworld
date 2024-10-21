#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include "options.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

namespace shader_utils {
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

} // namespace shader_utils

#endif