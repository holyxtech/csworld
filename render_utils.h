#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#define GLM_FORCE_LEFT_HANDED
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "types.h"
#include "voxel.h"

template <typename T, std::size_t LL, std::size_t RL>
constexpr std::array<T, LL + RL> join(const std::array<T, LL>& rhs, const std::array<T, RL>& lhs) {
  std::array<T, LL + RL> ar;
  auto current = std::copy(rhs.begin(), rhs.end(), ar.begin());
  std::copy(lhs.begin(), lhs.end(), current);

  return ar;
}

namespace RenderUtils {
  void preload_include(const std::string& path, const std::string& name);
  void create_shader(GLuint* shader, const std::string& vertex_shader_path, const std::string& fragment_shader_path);

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
  };

  constexpr auto named_irregular_textures = std::array{
    std::pair{"standing_grass", VT{IrregularTexture::standing_grass}},
    std::pair{"roses", VT{IrregularTexture::roses}},
    std::pair{"sunflower", VT{IrregularTexture::sunflower}}};

  constexpr auto named_textures = join(named_cube_textures, named_irregular_textures);

  // constexpr auto named_textures = std::array{std::pair{"asd", CubeTexture::dirt}};

} // namespace RenderUtils

#endif