#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <GL/glew.h>

class Shader {
public:
  Shader() = default;
  Shader(
    GLuint id,
    std::unordered_map<std::string, unsigned int>&& texture_binding_points,
    std::unordered_map<std::string, unsigned int>&& uniform_buffer_binding_points);
  GLuint get_id() const;
  const std::unordered_map<std::string, unsigned int>& get_texture_binding_points() const;
  const std::unordered_map<std::string, unsigned int>& get_uniform_buffer_binding_points() const;

private:
  GLuint id_;
  std::unordered_map<std::string, unsigned int> texture_binding_points_;
  std::unordered_map<std::string, unsigned int> uniform_buffer_binding_points_;
};

#endif