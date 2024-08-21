#include "shader.h"

Shader::Shader(
  GLuint id,
  std::unordered_map<std::string, unsigned int>&& texture_binding_points,
  std::unordered_map<std::string, unsigned int>&& uniform_buffer_binding_points)
    : id_(id),
      texture_binding_points_(std::move(texture_binding_points)),
      uniform_buffer_binding_points_(std::move(uniform_buffer_binding_points)) {}

GLuint Shader::get_id() const { return id_; }
const std::unordered_map<std::string, unsigned int>& Shader::get_texture_binding_points() const {
  return texture_binding_points_;
}
const std::unordered_map<std::string, unsigned int>& Shader::get_uniform_buffer_binding_points() const {
  return uniform_buffer_binding_points_;
}