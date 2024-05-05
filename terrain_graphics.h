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

namespace VertexKind {
  template <MeshGenerator::MeshKind mesh_kind>
  struct VertexKind {
    using type = Vertex;
  };

  template <>
  struct VertexKind<MeshGenerator::MeshKind::cubes> {
    using type = CubeVertex;
  };

  template <>
  struct VertexKind<MeshGenerator::MeshKind::irregular> {
    using type = Vertex;
  };

  template <>
  struct VertexKind<MeshGenerator::MeshKind::water> {
    using type = Vertex;
  };
} // namespace VertexKind

class TerrainGraphics {
public:
  TerrainGraphics();
  void render(const Renderer& renderer) const;
  void render_water(const Renderer& renderer) const;
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

  template <MeshGenerator::MeshKind mesh_kind>
  void upload(const Location& loc,
              const std::vector<typename VertexKind::VertexKind<mesh_kind>::type>& mesh,
              const Location& offset);
  void remove(const Location& loc, MultiDrawHandle& mdh);

  void render(const Renderer& renderer, const MultiDrawHandle& mdh) const;

  template <MeshGenerator::MeshKind mesh_kind>
  MultiDrawHandle& get_multi_draw_handle();

  template <typename T>
  void set_up_vao();

  template <MeshGenerator::MeshKind mesh_kind>
  void set_up();

  // specific for cubes
  MultiDrawHandle cubes_draw_handle_;
  GLuint cpx_ssbo_;
  GLuint cpy_ssbo_;
  GLuint cpz_ssbo_;

  // specific for irregular
  MultiDrawHandle irregular_draw_handle_;

  // specific for water
  MultiDrawHandle water_draw_handle_;
  GLuint normal_map1;
  GLuint normal_map2;

  // universal
  GLuint voxel_texture_array_;
};

#endif