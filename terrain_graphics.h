#ifndef TERRAIN_GRAPHICS_H
#define TERRAIN_GRAPHICS_H

#include <array>
#include <unordered_map>
#include <vector>
#include <GL/glew.h>
#include "lod_mesh_generator.h"
#include "mesh_generator.h"
#include "mesh_utils.h"
#include "region.h"
#include "types.h"

class Renderer;

enum class MeshKind {
  cubes,
  irregular,
  water,
  lod1,
  lod2,
  lod3
};

template <MeshKind mesh_kind>
struct VertexKind {
  using type = std::conditional_t<
    mesh_kind == MeshKind::cubes,
    CubeVertex,
    std::conditional_t<
      (mesh_kind == MeshKind::irregular || mesh_kind == MeshKind::water),
      Vertex,
      LodVertex>>;
};

class TerrainGraphics {
public:
  TerrainGraphics();
  void render(const Renderer& renderer) const;
  void render_water(const Renderer& renderer) const;
  void render_lods(const Renderer& renderer) const;
  void create(const Location& loc, const MeshGenerator& mesh_generator);
  void create(const Location& loc, LodLevel level, const LodMeshGenerator& lod_mesh_generator);
  void destroy(const Location& loc);
  void new_origin(const Location& loc);

private:
  static constexpr double lod_far_plane = 10000.;

  struct DrawArraysIndirectCommand {
    unsigned int count;
    unsigned int instance_count;
    unsigned int first;
    unsigned int base_instance;
  };

  struct CommandMetadata {
    unsigned int buffer_size;
  };

  struct MultiDrawHandle {
    GLuint shader;
    GLuint vbo;
    GLuint vao;
    GLuint ibo;
    std::vector<DrawArraysIndirectCommand> commands;
    std::vector<CommandMetadata> commands_metadata;
    int vbo_size = 0;
    std::unordered_map<Location, std::size_t, LocationHash> loc_to_command_index;
    std::size_t first_unoccupied = 0;
    GLuint loc_ssbo_;
  };

  template <MeshKind mesh_kind>
  void upload(
    const Location& loc,
    const std::vector<typename VertexKind<mesh_kind>::type>& mesh);
  void remove(const Location& loc, MultiDrawHandle& mdh);
  void render(const Renderer& renderer, const MultiDrawHandle& mdh) const;
  template <MeshKind mesh_kind>
  MultiDrawHandle& get_multi_draw_handle();
  template <typename T>
  void set_up_vao();
  template <MeshKind mesh_kind>
  void set_up();

  // specific for cubes
  MultiDrawHandle cubes_draw_handle_;

  // specific for irregular
  MultiDrawHandle irregular_draw_handle_;

  // specific for water
  MultiDrawHandle water_draw_handle_;
  GLuint normal_map1;
  GLuint normal_map2;

  // LODS
  MultiDrawHandle lod1_draw_handle_;
  glm::mat4 lod_projection_;

  GLuint voxel_texture_array_;
  GLuint lod_texture_array_;

  Location origin_;
};

#endif