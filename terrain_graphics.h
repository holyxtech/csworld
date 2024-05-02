#ifndef TERRAIN_GRAPHICS_H
#define TERRAIN_GRAPHICS_H

#include <array>
#include <unordered_map>
#include <vector>
#include "region.h"
#include "types.h"
#define GLM_FORCE_LEFT_HANDED
#include <GL/glew.h>

class Renderer;

class TerrainGraphics {
public:
  TerrainGraphics();
  void render(const Renderer& renderer) const;
  void create(const Location& loc, const std::vector<CubeVertex>& mesh, const Location& offset);
  void destroy(const Location& loc);

private:
  struct DrawArraysIndirectCommand {
    uint count;
    uint instance_count;
    uint first;
    uint base_instance;
  };

  struct CommandMetadata {
    bool occupied;
    uint buffer_size;
  };

  GLuint shader_;
  GLuint voxel_texture_array_;
  GLuint vbo_;
  GLuint vao_;
  GLuint ibo_;
  GLuint cpx_ssbo_;
  GLuint cpy_ssbo_;
  GLuint cpz_ssbo_;
  std::array<DrawArraysIndirectCommand, Region::max_sz> commands_;
  std::array<CommandMetadata, Region::max_sz> commands_metadata_;
  std::unordered_map<Location, std::size_t, LocationHash> loc_to_command_index_;

  int vbo_size_ = 0;
};

#endif