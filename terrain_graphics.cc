#include "terrain_graphics.h"
#include <cstddef>
#include "mesh_generator.h"
#include "region.h"
#include "render_utils.h"
#include "renderer.h"
#include "sky.h"
#include "stb_image.h"
#include "types.h"

TerrainGraphics::TerrainGraphics() {
  RenderUtils::create_shader(&shader_, "shaders/terrain.vs", "shaders/terrain.fs");
  glGenBuffers(1, &vbo_);
  glGenVertexArrays(1, &vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBindVertexArray(vao_);
  glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(CubeVertex), (void*)offsetof(CubeVertex, data_));
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(CubeVertex), (void*)offsetof(CubeVertex, lighting_));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  vbo_size_ = sizeof(CubeVertex) * MeshGenerator::defacto_vertices_per_mesh * Region::max_sz;
  glBufferData(GL_ARRAY_BUFFER, vbo_size_, nullptr, GL_STATIC_DRAW);

  for (int idx = 0; idx < commands_.size(); ++idx) {
    auto& command = commands_[idx];
    command.count = 0;
    command.instance_count = 1;
    command.first = idx * MeshGenerator::defacto_vertices_per_mesh;
    command.base_instance = 0;
    auto& metadata = commands_metadata_[idx];
    metadata.buffer_size = sizeof(CubeVertex) * MeshGenerator::defacto_vertices_per_mesh;
    metadata.occupied = false;
  }
  glGenBuffers(1, &ibo_);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ibo_);
  glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawArraysIndirectCommand) * commands_.size(), commands_.data(), GL_STATIC_DRAW);

  glCreateBuffers(1, &cpx_ssbo_);
  glNamedBufferStorage(cpx_ssbo_, sizeof(glm::vec3) * commands_.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
  glCreateBuffers(1, &cpy_ssbo_);
  glNamedBufferStorage(cpy_ssbo_, sizeof(glm::vec3) * commands_.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
  glCreateBuffers(1, &cpz_ssbo_);
  glNamedBufferStorage(cpz_ssbo_, sizeof(glm::vec3) * commands_.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

  glGenTextures(1, &voxel_texture_array_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, voxel_texture_array_);
  GLint num_layers = static_cast<GLint>(VoxelTexture::num_voxel_textures);
  GLsizei width = 16;
  GLsizei height = 16;
  GLsizei channels;
  GLsizei num_mipmaps = 1;
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, num_mipmaps, GL_SRGB8_ALPHA8, width, height, num_layers);
  stbi_set_flip_vertically_on_load(1);
  std::vector<std::pair<std::string, VoxelTexture>> textures = {
    std::make_pair("dirt", VoxelTexture::dirt),
    std::make_pair("grass", VoxelTexture::grass),
    std::make_pair("grass_side", VoxelTexture::grass_side),
    std::make_pair("water", VoxelTexture::water),
    std::make_pair("sand", VoxelTexture::sand),
    std::make_pair("tree_trunk", VoxelTexture::tree_trunk),
    std::make_pair("leaves", VoxelTexture::leaves),
    std::make_pair("sandstone", VoxelTexture::sandstone),
    std::make_pair("stone", VoxelTexture::stone),
    std::make_pair("standing_grass", VoxelTexture::standing_grass),
  };
  for (auto [filename, texture] : textures) {
    std::string path = "images/" + filename + ".png";
    auto* image_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(texture), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
  }
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glUseProgram(shader_);
  GLint texture_loc = glGetUniformLocation(shader_, "textureArray");
  glUniform1i(texture_loc, 0);
}

void TerrainGraphics::create(const Location& loc, const std::vector<CubeVertex>& mesh, const Location& offset) {

  std::size_t idx;
  if (loc_to_command_index_.contains(loc)) {
    idx = loc_to_command_index_[loc];
  } else {
    for (int i = 0; i < commands_.size(); ++i) {
      auto& metadata = commands_metadata_[i];
      if (!metadata.occupied) {
        idx = i;
        metadata.occupied = true;
        break;
      }
    }

    float chunk_pos_x = (loc[0] - offset[0]) * Chunk::sz_x;
    float chunk_pos_y = (loc[1] - offset[1]) * Chunk::sz_y;
    float chunk_pos_z = (loc[2] - offset[2]) * Chunk::sz_z;

    int ssbo_float_offset = sizeof(float) * idx;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cpx_ssbo_);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, ssbo_float_offset, sizeof(float), &chunk_pos_x);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cpy_ssbo_);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, ssbo_float_offset, sizeof(float), &chunk_pos_y);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cpz_ssbo_);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, ssbo_float_offset, sizeof(float), &chunk_pos_z);
  }

  auto& command = commands_[idx];
  auto& metadata = commands_metadata_[idx];

  GLuint mesh_size_bytes = sizeof(CubeVertex) * mesh.size();
  if (mesh_size_bytes > metadata.buffer_size) {
    std::cout << "Buffer too small for mesh with size " << mesh.size() << std::endl;

    GLuint new_vbo;
    glGenBuffers(1, &new_vbo);

    glBindBuffer(GL_COPY_READ_BUFFER, vbo_);
    glBindBuffer(GL_COPY_WRITE_BUFFER, new_vbo);

    // need to make sure added_size is a multiple of sizeof(CubeVertex)
    int added_size = (mesh_size_bytes - metadata.buffer_size) + (metadata.buffer_size * .05);
    added_size /= sizeof(CubeVertex);
    added_size *= sizeof(CubeVertex);
    glBufferData(GL_COPY_WRITE_BUFFER, vbo_size_ + added_size, nullptr, GL_STATIC_DRAW);

    // Copy beginning of old buffer
    int pre_size = command.first * sizeof(CubeVertex);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, pre_size);

    // Copy this mesh
    glBufferSubData(GL_COPY_WRITE_BUFFER, pre_size, mesh_size_bytes, mesh.data());

    if (idx < commands_.size() - 1) {
      // Copy end of old buffer
      int post_size = pre_size + metadata.buffer_size + added_size;

      glCopyBufferSubData(
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        pre_size + metadata.buffer_size,
        post_size,
        vbo_size_ - metadata.buffer_size - pre_size);
    }

    vbo_size_ += added_size;
    metadata.buffer_size += added_size;

    // Shift the first index of every command following this one
    for (int i = idx + 1; i < commands_.size(); ++i) {
      auto& command = commands_[i];
      command.first += (added_size / sizeof(CubeVertex));
    }
    command.count = mesh.size();

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ibo_);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, idx * sizeof(DrawArraysIndirectCommand), sizeof(DrawArraysIndirectCommand) * (commands_.size() - idx), commands_.data() + idx);

    glDeleteBuffers(1, &vbo_);
    vbo_ = new_vbo;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindVertexArray(vao_);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(CubeVertex), (void*)offsetof(CubeVertex, data_));
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(CubeVertex), (void*)offsetof(CubeVertex, lighting_));

  } else {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(CubeVertex) * command.first, mesh_size_bytes, mesh.data());

    command.count = mesh.size();
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ibo_);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawArraysIndirectCommand) * idx, sizeof(DrawArraysIndirectCommand), &command);
  }

  loc_to_command_index_[loc] = idx;
}

void TerrainGraphics::destroy(const Location& loc) {
  auto idx = loc_to_command_index_[loc];
  auto& command = commands_[idx];
  auto& metadata = commands_metadata_[idx];
  command.count = 0;
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ibo_);
  glBufferSubData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawArraysIndirectCommand) * idx, sizeof(DrawArraysIndirectCommand), &command);
  loc_to_command_index_.erase(loc);
  metadata.occupied = false;
}

void TerrainGraphics::render(const Renderer& renderer) const {
  glUseProgram(shader_);
  auto transform_loc = glGetUniformLocation(shader_, "uTransform");
  auto& projection = renderer.get_projection_matrix();
  auto& view = renderer.get_view_matrix();
  auto transform = projection * view;
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
  auto camera_world_position_loc = glGetUniformLocation(shader_, "uCameraWorldPosition");
  auto& camera_world_position = renderer.get_camera_world_position();
  glUniform3fv(camera_world_position_loc, 1, glm::value_ptr(camera_world_position));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, voxel_texture_array_);
  GLuint voxel_textures_loc = glGetUniformLocation(shader_, "textureArray");
  glUniform1i(voxel_textures_loc, 0);
  const Sky& sky = renderer.get_sky();
  GLuint texture = sky.get_texture();
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  glUniform1i(glGetUniformLocation(shader_, "skybox"), 1);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cpx_ssbo_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, cpy_ssbo_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, cpz_ssbo_);
  glBindVertexArray(vao_);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ibo_);
  glMultiDrawArraysIndirect(GL_TRIANGLES, 0, commands_.size(), 0);
}