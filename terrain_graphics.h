#ifndef TERRAIN_GRAPHICS_H
#define TERRAIN_GRAPHICS_H

#include <array>
#include <unordered_map>
#include <vector>
#include "mesh_generator.h"
#include "region.h"
#include "types.h"
#define GLM_FORCE_LEFT_HANDED
#include <GL/glew.h>

class Renderer;

class TerrainGraphics {
public:
  TerrainGraphics();
  void render(const Renderer& renderer) const;
  void create(const Location& loc, const MeshGenerator& mesh_generator);

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

  struct MultiDrawHandle {
    GLuint shader;
    GLuint vbo;
    GLuint vao;
    GLuint ibo;
    std::array<DrawArraysIndirectCommand, Region::max_sz> commands;
    std::array<CommandMetadata, Region::max_sz> commands_metadata;
    int vbo_size = 0;
    std::unordered_map<Location, std::size_t, LocationHash> loc_to_command_index;
  };

  template <typename T>
  void upload(const Location& loc, const std::vector<T>& mesh, const Location& offset);
  
  void remove(const Location& loc, MultiDrawHandle& mdh);

  template <typename T>
  MultiDrawHandle& get_multi_draw_handle();

  template <typename T>
  void set_up_vao();

  template <typename T>
  void set_up();

  // specific for cubes
  MultiDrawHandle cubes_draw_handle_;
  GLuint cpx_ssbo_;
  GLuint cpy_ssbo_;
  GLuint cpz_ssbo_;

  // specific for irregular
  MultiDrawHandle irregular_draw_handle_;

  // universal
  GLuint voxel_texture_array_;
};

#endif