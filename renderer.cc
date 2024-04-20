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
  RenderUtils::create_shader(&shader_, "shaders/terrain.vs", "shaders/terrain.fs");
  RenderUtils::create_shader(&composite_shader_, "shaders/composite.vs", "shaders/composite.fs");
  RenderUtils::create_shader(&voxel_highlight_shader_, "shaders/voxel_highlight.vs", "shaders/voxel_highlight.fs");
  RenderUtils::create_shader(&blur_shader_, "shaders/blur.vs", "shaders/blur.fs");
  RenderUtils::create_shader(&final_shader_, "shaders/final.vs", "shaders/final.fs");

  std::array<GLuint, Region::max_sz> vbos;
  std::array<GLuint, Region::max_sz> vaos;
  glGenBuffers(vbos.size(), vbos.data());
  glGenVertexArrays(vaos.size(), vaos.data());
  for (int i = 0; i < vbos.size(); ++i) {
    auto vbo = vbos[i];
    auto vao = vaos[i];
    set_up_terrain_vao(vbo, vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MeshGenerator::default_max_vertices, nullptr, GL_STATIC_DRAW);
    vbo_to_vao_[vbo] = vao;
  }
  glGenBuffers(vbos.size(), vbos.data());
  glGenVertexArrays(vaos.size(), vaos.data());
  for (int i = 0; i < vbos.size(); ++i) {
    auto vbo = vbos[i];
    auto vao = vaos[i];
    set_up_terrain_vao(vbo, vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MeshGenerator::default_max_water_vertices, nullptr, GL_STATIC_DRAW);
    water_vbo_to_vao_[vbo] = vao;
  }

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

  float quad[] = {
    -1.0f, -1.0f,
    -1.0f, 1.0f,
    1.0f, -1.0f,
    -1.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, -1.0f};
  glGenBuffers(1, &quad_vbo_);
  glGenVertexArrays(1, &quad_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
  glBindVertexArray(quad_vao_);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  auto set_up_framebuffers = [](GLuint& fbo, GLuint& cbo, GLuint& dbo) -> void {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &cbo);
    glBindTexture(GL_TEXTURE_2D, cbo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cbo, 0);

    glGenTextures(1, &dbo);
    glBindTexture(GL_TEXTURE_2D, dbo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbo, 0);
  };
  set_up_framebuffers(main_framebuffer_, main_cbo_, main_dbo_);
  set_up_framebuffers(water_framebuffer_, water_cbo_, water_dbo_);
  glGenFramebuffers(1, &composite_framebuffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, composite_framebuffer_);
  glGenTextures(2, composite_cbos_.data());
  for (size_t i = 0; i < 2; ++i) {
    glBindTexture(GL_TEXTURE_2D, composite_cbos_[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, composite_cbos_[i], 0);
  }
  std::array<GLuint, 2> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments.data());

  glGenFramebuffers(2, pingpong_framebuffers_.data());
  glGenTextures(2, pingpong_textures_.data());
  for (size_t i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, pingpong_framebuffers_[i]);
    glBindTexture(GL_TEXTURE_2D, pingpong_textures_[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, blur_texture_width_, blur_texture_height_, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpong_textures_[i], 0);
  }

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glLineWidth(4.f);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
  glClearColor(0.702f, 0.266f, 1.f, 1.0f);

  projection_ = glm::perspective(glm::radians(45.l), 16 / 9.l, 0.1l, 4000.l);
}

void Renderer::set_up_terrain_vao(GLuint vbo, GLuint vao) const {
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindVertexArray(vao);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvs));
  glVertexAttribPointer(2, 1, GL_INT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, layer));
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, lighting));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
}

void Renderer::creation(
  const Location& loc,
  const std::vector<Vertex>& mesh,
  std::unordered_map<GLuint, GLuint>& vbo_to_vao,
  std::unordered_map<GLuint, Location>& vbo_map,
  std::unordered_map<Location, GLuint, LocationHash>& loc_map,
  std::unordered_map<GLuint, int>& mesh_size_map) {

  GLuint vbo_to_use = 0;
  GLuint vao_to_use = 0;

  if (loc_map.contains(loc)) {
    vbo_to_use = loc_map.at(loc);
    vao_to_use = vbo_to_vao.at(vbo_to_use);
  } else {
    for (auto [vbo, vao] : vbo_to_vao) {
      if (!vbo_map.contains(vbo)) {
        vbo_to_use = vbo;
        vao_to_use = vao;
        break;
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo_to_use);
  GLint bufferSize;
  glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
  if (bufferSize < sizeof(Vertex) * mesh.size()) {
    std::cout << "Buffer too small! This mesh has size " << mesh.size() << std::endl;

    glDeleteBuffers(1, &vbo_to_use);
    glGenBuffers(1, &vbo_to_use);
    set_up_terrain_vao(vbo_to_use, vao_to_use);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh.size(), mesh.data(), GL_STATIC_DRAW);
  } else {
    GLint mesh_size_bytes = sizeof(Vertex) * mesh.size();
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh_size_bytes, mesh.data());
    GLint remaining_bytes = bufferSize - mesh_size_bytes;
    // Fill the remaining bytes with zeros starting from mesh size
    glBufferSubData(GL_ARRAY_BUFFER, mesh_size_bytes, remaining_bytes, nullptr);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  vbo_map[vbo_to_use] = loc;
  loc_map[loc] = vbo_to_use;
  mesh_size_map[vbo_to_use] = mesh.size();
}

void Renderer::consume_mesh_generator(MeshGenerator& mesh_generator) {
  auto& diffs = mesh_generator.get_diffs();
  for (auto& diff : diffs) {
    if (diff.kind == MeshGenerator::Diff::creation) {
      auto& loc = diff.location;
      creation(loc, mesh_generator.get_meshes().at(loc), vbo_to_vao_, vbo_map_, loc_map_, mesh_size_map_);
      creation(loc, mesh_generator.get_water_meshes().at(loc), water_vbo_to_vao_, water_vbo_map_, water_loc_map_, water_mesh_size_map_);

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
  camera_world_position_ = camera.get_world_position(camera_offset_);
}

void Renderer::render() const {
  glUseProgram(shader_);
  auto transform_loc = glGetUniformLocation(shader_, "uTransform");
  auto transform = projection_ * view_;
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &transform[0][0]);
  auto camera_world_position_loc = glGetUniformLocation(shader_, "uCameraWorldPosition");
  glUniform3fv(camera_world_position_loc, 1, glm::value_ptr(camera_world_position_));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, voxel_texture_array_);
  GLuint voxel_textures_loc = glGetUniformLocation(shader_, "textureArray");
  glUniform1i(voxel_textures_loc, 0);
  GLuint texture = sky_.get_texture();
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  glUniform1i(glGetUniformLocation(shader_, "skybox"), 1);

  glEnable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, water_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for (auto [vbo, vao] : water_vbo_to_vao_) {
    if (!(water_vbo_map_.contains(vbo) && water_mesh_size_map_.at(vbo) > 0))
      continue;
    glBindVertexArray(vao);
    int mesh_size = water_mesh_size_map_.at(vbo);
    glDrawArrays(GL_TRIANGLES, 0, mesh_size);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, main_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for (auto [vbo, vao] : vbo_to_vao_) {
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
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
  glDisable(GL_DEPTH_TEST);

  glBindFramebuffer(GL_FRAMEBUFFER, composite_framebuffer_);
  glUseProgram(composite_shader_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, main_cbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "mainColor"), 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, water_cbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterColor"), 1);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, main_dbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "mainDepth"), 2);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, water_dbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterDepth"), 3);
  glBindVertexArray(quad_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // apply bloom...
  glUseProgram(blur_shader_);
  glViewport(0, 0, blur_texture_width_, blur_texture_height_);
  glActiveTexture(GL_TEXTURE4);
  glUniform1i(glGetUniformLocation(blur_shader_, "image"), 4);
  bool horizontal = true, first_iteration = true;
  int amount = 3;
  for (size_t i = 0; i < amount; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, pingpong_framebuffers_[horizontal]);
    glUniform1i(glGetUniformLocation(blur_shader_, "horizontal"), horizontal);
    glBindTexture(GL_TEXTURE_2D, first_iteration ? composite_cbos_[1] : pingpong_textures_[!horizontal]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    horizontal = !horizontal;
    first_iteration = false;
  }

  glViewport(0, 0, window_width, window_height);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(final_shader_);
  glBindVertexArray(quad_vao_);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, composite_cbos_[0]);
  glUniform1i(glGetUniformLocation(final_shader_, "scene"), 5);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, pingpong_textures_[!horizontal]);
  glUniform1i(glGetUniformLocation(final_shader_, "bloom"), 6);
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

float Renderer::normalize_x(float x) {
  return x * (window_height / static_cast<float>(window_width));
}