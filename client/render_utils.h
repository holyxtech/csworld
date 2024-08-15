#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "types.h"
#include "voxel.h"

namespace RenderUtils {
  template <typename T, std::size_t LL, std::size_t RL>
  constexpr std::array<T, LL + RL> join(const std::array<T, LL>& rhs, const std::array<T, RL>& lhs) {
    std::array<T, LL + RL> ar;
    auto current = std::copy(rhs.begin(), rhs.end(), ar.begin());
    std::copy(lhs.begin(), lhs.end(), current);
    return ar;
  }
  constexpr auto named_cube_textures = std::array{
    std::pair{"dirt", VT{CubeTexture::dirt}},
    std::pair{"grass", VT{CubeTexture::grass}},
    std::pair{"grass_side", VT{CubeTexture::grass_side}},
    std::pair{"water", VT{CubeTexture::water}},
    std::pair{"sand", VT{CubeTexture::sand}},
    std::pair{"tree_trunk", VT{CubeTexture::tree_trunk}},
    std::pair{"leaves", VT{CubeTexture::leaves}},
    std::pair{"sandstone", VT{CubeTexture::sandstone}},
    std::pair{"stone", VT{CubeTexture::stone}},
    std::pair{"bricks", VT{CubeTexture::bricks}}};
  constexpr auto named_irregular_textures = std::array{
    std::pair{"standing_grass", VT{IrregularTexture::standing_grass}},
    std::pair{"roses", VT{IrregularTexture::roses}},
    std::pair{"sunflower", VT{IrregularTexture::sunflower}}};
  constexpr auto named_textures = join(named_cube_textures, named_irregular_textures);

  GLuint create_shader(const std::string& vertex_shader_name, const std::string& fragment_shader_name);
  GLuint create_shader(const std::string& vertex_shader_name, const std::string& geometry_shader_name, const std::string& fragment_shader_name);
  GLuint create_shader(const std::string& compute_shader_name);
  std::vector<glm::vec3> get_frustum_corners_world_space(const glm::mat4& proj, const glm::mat4& view);

} // namespace RenderUtils

#endif