#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <fstream>
#include <iostream>
#include <string>
#define GLM_FORCE_LEFT_HANDED
#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "types.h"

namespace RenderUtils {
  void preload_include(const std::string& path, const std::string& name);
  void create_shader(GLuint* shader, const std::string& vertex_shader_path, const std::string& fragment_shader_path);
} // namespace RenderUtils

#endif