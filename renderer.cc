#include "renderer.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <utility>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "options.h"
#include "render_utils.h"
#include "stb_image.h"
#include "types.h"

Renderer::Renderer(World& world) : world_(world) {
  projection_ = glm::perspective(glm::radians(45.), 16 / 9., 0.1, region_far_plane);

  RenderUtils::preload_include(Options::instance()->getShaderPath("ssr.glsl"), "/ssr.glsl");

  RenderUtils::create_shader(&composite_shader_, Options::instance()->getShaderPath("composite.vs"), Options::instance()->getShaderPath("composite.fs"));
  RenderUtils::create_shader(&voxel_highlight_shader_, Options::instance()->getShaderPath("voxel_highlight.vs"), Options::instance()->getShaderPath("voxel_highlight.fs"));
  RenderUtils::create_shader(&blur_shader_, Options::instance()->getShaderPath("blur.vs"), Options::instance()->getShaderPath("blur.fs"));
  RenderUtils::create_shader(&final_shader_, Options::instance()->getShaderPath("final.vs"), Options::instance()->getShaderPath("final.fs"));

  float voxel_highlight_vertices[] = {
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f};
  unsigned int indices[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cbo, 0);

    glGenTextures(1, &dbo);
    glBindTexture(GL_TEXTURE_2D, dbo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbo, 0);
  };
  set_up_framebuffers(main_framebuffer_, main_cbo_, main_dbo_);
  set_up_framebuffers(water_framebuffer_, water_cbo_, water_dbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, water_framebuffer_);
  std::array<std::reference_wrapper<GLuint>, 2> g_bufs = {std::ref(g_water_position_), std::ref(g_water_normal_)};
  for (size_t i = 0; i < 2; ++i) {
    auto& g_buf = g_bufs[i].get();
    glGenTextures(1, &g_buf);
    glBindTexture(GL_TEXTURE_2D, g_buf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 + i, GL_TEXTURE_2D, g_buf, 0);
  }
  {
    std::array<GLuint, 3> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments.data());
  }

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
  {
    std::array<GLuint, 2> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments.data());
  }

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

  float sx = float(window_width) / 2.0;
  float sy = float(window_height) / 2.0;
  auto warp_to_screen_space = glm::mat4(1.0);
  warp_to_screen_space[0] = glm::vec4(sx, 0, 0, sx);
  warp_to_screen_space[1] = glm::vec4(0, sy, 0, sy);
  warp_to_screen_space = glm::transpose(warp_to_screen_space);
  glm::mat4 pixel_projection = warp_to_screen_space * projection_;
  glUseProgram(composite_shader_);
  auto pixel_projection_loc = glGetUniformLocation(composite_shader_, "uPixelProjection");
  glUniformMatrix4fv(pixel_projection_loc, 1, GL_FALSE, glm::value_ptr(pixel_projection));

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glLineWidth(4.f);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
  glClearColor(0.f, 0.f, 0.f, 1.0f);
}

void Renderer::consume_mesh_generator(MeshGenerator& mesh_generator) {
  auto& diffs = mesh_generator.get_diffs();
  for (auto& diff : diffs) {
    auto& loc = diff.location;
    if (diff.kind == MeshGenerator::Diff::creation) {
      terrain_.create(loc, mesh_generator);
    } else if (diff.kind == MeshGenerator::Diff::deletion) {
      terrain_.destroy(loc);
    } else if (diff.kind == MeshGenerator::Diff::origin) {
      camera_offset_ = glm::dvec3(loc[0] * Chunk::sz_x, loc[1] * Chunk::sz_y, loc[2] * Chunk::sz_z);
      terrain_.new_origin(loc);
    }
  }

  mesh_generator.clear_diffs();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::consume_lod_mesh_generator(LodMeshGenerator& lod_mesh_generator) {
  auto& diffs = lod_mesh_generator.get_diffs();
  for (auto& diff : diffs) {
    auto& loc = diff.location;
    if (diff.kind == LodMeshGenerator::Diff::creation) {
      auto level = std::any_cast<const LodMeshGenerator::Diff::CreationData&>(diff.data).level;
      terrain_.create(loc, level, lod_mesh_generator);
    }
  }
  lod_mesh_generator.clear_diffs();
}

void Renderer::consume_camera(const Camera& camera) {
  view_ = camera.get_view(camera_offset_);
  camera_world_position_ = camera.get_world_position(camera_offset_);
}

void Renderer::render() const {

  glEnable(GL_DEPTH_TEST);

  glBindFramebuffer(GL_FRAMEBUFFER, water_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  terrain_.render_water(*this);

  glBindFramebuffer(GL_FRAMEBUFFER, main_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  terrain_.render_lods(*this);
  sky_.render(*this);
  glClear(GL_DEPTH_BUFFER_BIT);
  terrain_.render(*this);

  glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, composite_framebuffer_);
  glUseProgram(composite_shader_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, main_cbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "mainColor"), 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, main_dbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "mainDepth"), 1);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, water_cbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterColor"), 2);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, water_dbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterDepth"), 3);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, g_water_position_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterPosition"), 4);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, g_water_normal_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterNormal"), 5);
  glBindVertexArray(quad_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // apply bloom...
  glUseProgram(blur_shader_);
  glViewport(0, 0, blur_texture_width_, blur_texture_height_);
  glActiveTexture(GL_TEXTURE6);
  glUniform1i(glGetUniformLocation(blur_shader_, "image"), 6);
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
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, composite_cbos_[0]);
  glUniform1i(glGetUniformLocation(final_shader_, "scene"), 7);
  glActiveTexture(GL_TEXTURE8);
  glBindTexture(GL_TEXTURE_2D, pingpong_textures_[!horizontal]);
  glUniform1i(glGetUniformLocation(final_shader_, "bloom"), 8);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // UI
  glUseProgram(voxel_highlight_shader_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, main_dbo_);
  glUniform1i(glGetUniformLocation(voxel_highlight_shader_, "depth"), 0);
  auto transform_loc = glGetUniformLocation(voxel_highlight_shader_, "uTransform");
  auto transform = projection_ * view_;
  transform = projection_ * view_ * glm::translate(glm::mat4(1.f), voxel_highlight_position_) * glm::scale(glm::mat4(1.f), glm::vec3(1.001));
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
  glBindVertexArray(voxel_highlight_vao_);
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

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

const Sky& Renderer::get_sky() const {
  return sky_;
}

const glm::vec3 Renderer::get_camera_world_position() const {
  return camera_world_position_;
}
