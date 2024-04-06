#include "renderer.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <utility>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "render_utils.h"
#include "stb_image.h"
#include "types.h"

Renderer::Renderer(World& world) : world_(world) {
  RenderUtils::create_shader(&shader_, "shaders/vertex.glsl", "shaders/fragment.glsl");
  RenderUtils::create_shader(&window_shader_, "shaders/window_vertex.glsl", "shaders/window_fragment.glsl");
  RenderUtils::create_shader(&voxel_highlight_shader_, "shaders/voxel_highlight_vertex.glsl", "shaders/voxel_highlight_fragment.glsl");

  std::array<GLuint, Region::max_sz> vbos;
  std::array<GLuint, Region::max_sz> vaos;
  glGenBuffers(vbos.size(), vbos.data());
  glGenVertexArrays(vaos.size(), vaos.data());
  for (int i = 0; i < vbos.size(); ++i) {
    auto vbo = vbos[i];
    auto vao = vaos[i];
    RenderUtils::set_up_standard_vao(vbo, vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MeshGenerator::default_max_vertices, nullptr, GL_STATIC_DRAW);
    vbo_to_vao_[vbo] = vao;
  }
  glGenBuffers(vbos.size(), vbos.data());
  glGenVertexArrays(vaos.size(), vaos.data());
  for (int i = 0; i < vbos.size(); ++i) {
    auto vbo = vbos[i];
    auto vao = vaos[i];
    RenderUtils::set_up_standard_vao(vbo, vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MeshGenerator::default_max_water_vertices, nullptr, GL_STATIC_DRAW);
    water_vbo_to_vao_[vbo] = vao;
  }

  GLuint texture_array;
  glGenTextures(1, &texture_array);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
  GLint num_layers = static_cast<GLint>(VoxelTexture::num_voxel_textures);
  GLsizei width = 16;
  GLsizei height = 16;
  GLsizei num_mipmaps = 1;
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, num_mipmaps, GL_RGBA8, width, height, num_layers);
  stbi_set_flip_vertically_on_load(1);
  int _width, _height, channels;
  auto* image_data = stbi_load("images/dirt.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::dirt), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/grass.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::grass), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/grass_side.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::grass_side), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/water.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::water), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/sand.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::sand), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/tree_trunk.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::tree_trunk), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/leaves.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::leaves), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/sandstone.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::sandstone), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/stone.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::stone), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  image_data = stbi_load("images/standing_grass.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(VoxelTexture::standing_grass), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);
  // glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY, 4.0);
  glUseProgram(shader_);
  GLint texture_loc = glGetUniformLocation(shader_, "textureArray");
  glUniform1i(texture_loc, 0);

  float window_vertices[] = {
    -1.0f, -1.0f,
    -1.0f, 1.0f,
    1.0f, -1.0f,
    -1.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, -1.0f};
  glGenBuffers(1, &window_vbo_);
  glGenVertexArrays(1, &window_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, window_vbo_);
  glBindVertexArray(window_vao_);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);
  glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertices), window_vertices, GL_STATIC_DRAW);
  auto set_up_framebuffers = [](GLuint* fbo, GLuint* cbo, GLuint* dbo) {
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
    glGenTextures(1, cbo);
    glGenTextures(1, dbo);
    glBindTexture(GL_TEXTURE_2D, *cbo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, *dbo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *cbo, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *dbo, 0);
  };
  set_up_framebuffers(&main_framebuffer_, &main_cbo_, &main_dbo_);
  set_up_framebuffers(&water_framebuffer_, &water_cbo_, &water_dbo_);
  glUseProgram(window_shader_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, main_cbo_);
  glUniform1i(glGetUniformLocation(window_shader_, "MainColor"), 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, water_cbo_);
  glUniform1i(glGetUniformLocation(window_shader_, "WaterColor"), 1);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, main_dbo_);
  glUniform1i(glGetUniformLocation(window_shader_, "MainDepth"), 2);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, water_dbo_);
  glUniform1i(glGetUniformLocation(window_shader_, "WaterDepth"), 3);

  float voxel_highlight_vertices[] = {
    // Front face
    0.0f, 0.0f, 1.0f, // Bottom-left
    1.0f, 0.0f, 1.0f, // Bottom-right
    1.0f, 1.0f, 1.0f, // Top-right
    0.0f, 1.0f, 1.0f, // Top-left
    // Back face
    0.0f, 0.0f, 0.0f, // Bottom-left
    1.0f, 0.0f, 0.0f, // Bottom-right
    1.0f, 1.0f, 0.0f, // Top-right
    0.0f, 1.0f, 0.0f  // Top-left
  };

  unsigned int indices[] = {
    // Front face
    0, 1, 1, 2, 2, 3, 3, 0,
    // Back face
    4, 5, 5, 6, 6, 7, 7, 4,
    // Connecting lines
    0, 4, 1, 5, 2, 6, 3, 7};

  GLuint voxel_highlight_ebo;
  glGenVertexArrays(1, &voxel_highlight_vao_);
  glGenBuffers(1, &voxel_highlight_vbo_);
  glGenBuffers(1, &voxel_highlight_ebo);
  glBindVertexArray(voxel_highlight_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, voxel_highlight_vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(voxel_highlight_vertices), voxel_highlight_vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxel_highlight_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glLineWidth(4.f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
  glClearColor(0.702f, 0.266f, 1.f, 1.0f);

  projection_ = glm::perspective(glm::radians(45.l), 16 / 9.l, 0.1l, 4000.l);
}

void Renderer::creation(
  const Location& loc,
  const std::vector<Vertex>* mesh,
  std::unordered_map<GLuint, GLuint>* vbo_to_vao,
  std::unordered_map<GLuint, Location>* vbo_map,
  std::unordered_map<Location, GLuint, LocationHash>* loc_map,
  std::unordered_map<GLuint, int>* mesh_size_map) {

  GLuint vbo_to_use = 0;
  GLuint vao_to_use = 0;

  if (loc_map->contains(loc)) {
    vbo_to_use = loc_map->at(loc);
    vao_to_use = vbo_to_vao->at(vbo_to_use);
  } else {
    for (auto& [vbo, vao] : *vbo_to_vao) {
      if (!vbo_map->contains(vbo)) {
        vbo_to_use = vbo;
        vao_to_use = vao;
        break;
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo_to_use);
  GLint bufferSize;
  glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
  if (bufferSize < sizeof(Vertex) * mesh->size()) {
    std::cout << "Buffer too small! This mesh has size " << mesh->size() << std::endl;

    glDeleteBuffers(1, &vbo_to_use);
    glGenBuffers(1, &vbo_to_use);
    RenderUtils::set_up_standard_vao(vbo_to_use, vao_to_use);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh->size(), mesh->data(), GL_STATIC_DRAW);
  } else {
    GLint mesh_size_bytes = sizeof(Vertex) * mesh->size();
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh_size_bytes, mesh->data());
    GLint remaining_bytes = bufferSize - mesh_size_bytes;
    // Fill the remaining bytes with zeros starting from mesh size
    glBufferSubData(GL_ARRAY_BUFFER, mesh_size_bytes, remaining_bytes, nullptr);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  (*vbo_map)[vbo_to_use] = loc;
  (*loc_map)[loc] = vbo_to_use;
  (*mesh_size_map)[vbo_to_use] = mesh->size();
}

void Renderer::consume_mesh_generator(MeshGenerator& mesh_generator) {
  auto& diffs = mesh_generator.get_diffs();
  for (auto& diff : diffs) {
    if (diff.kind == MeshGenerator::Diff::creation) {
      auto& loc = diff.location;
      creation(loc, &mesh_generator.get_meshes().at(loc), &vbo_to_vao_, &vbo_map_, &loc_map_, &mesh_size_map_);
      creation(loc, &mesh_generator.get_water_meshes().at(loc), &water_vbo_to_vao_, &water_vbo_map_, &water_loc_map_, &water_mesh_size_map_);

    } else if (diff.kind == MeshGenerator::Diff::deletion) {
      GLuint vbo = loc_map_[diff.location];
      vbo_map_.erase(vbo);
      loc_map_.erase(diff.location);
      GLuint water_vbo = water_loc_map_[diff.location];
      water_vbo_map_.erase(water_vbo);
      water_loc_map_.erase(diff.location);
    } else if (diff.kind == MeshGenerator::Diff::origin) {
      auto& loc = diff.location;
      camera_offset_ = glm::dvec3(loc[0] * Chunk::sz_x, loc[1] * Chunk::sz_y, loc[2] * Chunk::sz_z);
    }
  }

  mesh_generator.clear_diffs();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::consume_camera(const Camera& camera) {
  view_ = camera.get_view(camera_offset_);
}

void Renderer::render() const {

  glUseProgram(shader_);
  auto transform_loc = glGetUniformLocation(shader_, "uTransform");
  auto transform = projection_ * view_;
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &transform[0][0]);

  glUseProgram(shader_);
  glBindFramebuffer(GL_FRAMEBUFFER, water_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for (auto& [vbo, vao] : water_vbo_to_vao_) {
    if (!(water_vbo_map_.contains(vbo) && water_mesh_size_map_.at(vbo) > 0))
      continue;

    glBindVertexArray(vao);

    int mesh_size = water_mesh_size_map_.at(vbo);
    glDrawArrays(GL_TRIANGLES, 0, mesh_size);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, main_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for (auto& [vbo, vao] : vbo_to_vao_) {
    if (!vbo_map_.contains(vbo))
      continue;

    glBindVertexArray(vao);

    int mesh_size = mesh_size_map_.at(vbo);
    glDrawArrays(GL_TRIANGLES, 0, mesh_size);
  }

  sky_.render(*this);

  glUseProgram(voxel_highlight_shader_);
  transform_loc = glGetUniformLocation(voxel_highlight_shader_, "uTransform");
  transform = projection_ * view_ * glm::translate(voxel_highlight_position_) * glm::scale(glm::vec3(1.001));
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &transform[0][0]);
  glBindVertexArray(voxel_highlight_vao_);
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0); // 24 indices to draw lines for a cube

  glUseProgram(window_shader_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(window_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  ui_.render(*this);
}

const glm::mat4& Renderer::get_projection_matrix() const {
  return projection_;
}

const glm::mat4& Renderer::get_view_matrix() const {
  return view_;
}

void Renderer::set_highlight(Int3D& highlight) {
  voxel_highlight_position_ = glm::dvec3(highlight[0], highlight[1], highlight[2]) - camera_offset_;
}

void Renderer::consume_ui(UI& ui) {
  ui_.consume_ui(ui);
}