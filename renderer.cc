#include "renderer.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <utility>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "options.h"
#include "render_utils.h"
#include "stb_image.h"
#include "types.h"

int Renderer::window_width = 2560;
int Renderer::window_height = 1440;
double Renderer::aspect_ratio = Renderer::window_width / static_cast<double>(Renderer::window_height);
double Renderer::fov = glm::radians(45.);

Renderer::Renderer(GLFWwindow* window, const UI& ui) : window_(window), ui_graphics_(window, ui) {
  projection_ = glm::perspective(fov, aspect_ratio, near_plane, far_plane);

  composite_shader_ = RenderUtils::create_shader(Options::instance()->getShaderPath("composite.vs"), Options::instance()->getShaderPath("composite.fs"));
  voxel_highlight_shader_ = RenderUtils::create_shader(Options::instance()->getShaderPath("voxel_highlight.vs"), Options::instance()->getShaderPath("voxel_highlight.fs"));
  blur_shader_ = RenderUtils::create_shader(Options::instance()->getShaderPath("blur.vs"), Options::instance()->getShaderPath("blur.fs"));
  final_shader_ = RenderUtils::create_shader(Options::instance()->getShaderPath("final.vs"), Options::instance()->getShaderPath("final.fs"));

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
  GLuint voxel_highlight_vbo;
  glGenVertexArrays(1, &voxel_highlight_vao_);
  glGenBuffers(1, &voxel_highlight_vbo);
  glGenBuffers(1, &voxel_highlight_ebo);
  glBindVertexArray(voxel_highlight_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, voxel_highlight_vbo);
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
  GLuint quad_vbo;
  glGenBuffers(1, &quad_vbo);
  glGenVertexArrays(1, &quad_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
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
  glBindFramebuffer(GL_FRAMEBUFFER, main_framebuffer_);

  set_up_framebuffers(water_framebuffer_, water_cbo_, water_dbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, water_framebuffer_);
  std::array<std::reference_wrapper<GLuint>, 2> g_bufs = {std::ref(water_camera_position_), std::ref(water_camera_normal_)};
  for (std::size_t i = 0; i < 2; ++i) {
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
  for (std::size_t i = 0; i < 2; ++i) {
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
  for (std::size_t i = 0; i < 2; ++i) {
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

  // shadows
  glCreateBuffers(1, &light_space_matrices_ubo_);
  glNamedBufferStorage(light_space_matrices_ubo_, sizeof(glm::mat4) * num_cascades, nullptr, GL_DYNAMIC_STORAGE_BIT);
  glCreateBuffers(1, &shadow_block_ubo_);
  ShadowBlock sb;
  sb.far_plane = far_plane;
  sb.light_dir = sky_.get_sun_dir();
  double near = near_plane;
  for (int i = 0; i < num_cascades; ++i) {
    double far = near * view_plane_depth_ratio;
    sb.cascade_plane_distances[i / 4][i % 4] = far;
    near = far;
  }
  glNamedBufferStorage(shadow_block_ubo_, sizeof(ShadowBlock), &sb, GL_DYNAMIC_STORAGE_BIT);

  glGenFramebuffers(1, &shadow_fbo_);
  glGenTextures(1, &shadow_texture_);
  glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_texture_);
  glTexImage3D(
    GL_TEXTURE_2D_ARRAY,
    0,
    GL_DEPTH_COMPONENT32F,
    shadow_res,
    shadow_res,
    num_cascades,
    0,
    GL_DEPTH_COMPONENT,
    GL_FLOAT,
    nullptr);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  constexpr float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_texture_, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

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
}

void Renderer::consume_lod_mesh_generator(LodMeshGenerator& lod_mesh_generator) {
  auto& diffs = lod_mesh_generator.get_diffs();
  for (auto& diff : diffs) {
    auto& loc = diff.location;
    if (diff.kind == LodMeshGenerator::Diff::creation) {
      auto level = std::any_cast<const LodMeshGenerator::Diff::CreationData&>(diff.data).level;
    }
  }
  lod_mesh_generator.clear_diffs();
}

void Renderer::consume_camera(const Camera& camera) {
  view_ = camera.get_view(camera_offset_);
  camera_world_position_ = camera.get_world_position(camera_offset_);
}

void Renderer::render() {
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  // uniform blocks
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, light_space_matrices_ubo_);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, shadow_block_ubo_);

  shadow_map();

  glCullFace(GL_BACK);
  glViewport(0, 0, window_width, window_height);
  glBindFramebuffer(GL_FRAMEBUFFER, water_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  terrain_.render_water(*this);

  glBindFramebuffer(GL_FRAMEBUFFER, main_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  terrain_.render(*this);
  sky_.render(*this);

  glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, composite_framebuffer_);
  glUseProgram(composite_shader_);
  glBindTextureUnit(0, main_cbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "mainColor"), 0);
  glBindTextureUnit(1, main_dbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "mainDepth"), 1);
  glBindTextureUnit(2, water_cbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterColor"), 2);
  glBindTextureUnit(3, water_dbo_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterDepth"), 3);
  glBindTextureUnit(4, water_camera_position_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterCameraPosition"), 4);
  glBindTextureUnit(5, water_camera_normal_);
  glUniform1i(glGetUniformLocation(composite_shader_, "waterCameraNormal"), 5);
  glBindVertexArray(quad_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // apply bloom...
  bool horizontal = true;
  glNamedFramebufferReadBuffer(composite_framebuffer_, GL_COLOR_ATTACHMENT0 + 1);
  glBlitNamedFramebuffer(
    composite_framebuffer_, pingpong_framebuffers_[horizontal], 0, 0, window_width, window_height,
    0, 0, blur_texture_width_, blur_texture_height_, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glUseProgram(blur_shader_);
  glViewport(0, 0, blur_texture_width_, blur_texture_height_);
  glActiveTexture(GL_TEXTURE6);
  glUniform1i(glGetUniformLocation(blur_shader_, "image"), 6);
  for (int i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, pingpong_framebuffers_[!horizontal]);
    glUniform1i(glGetUniformLocation(blur_shader_, "horizontal"), !horizontal);
    glBindTexture(GL_TEXTURE_2D, pingpong_textures_[horizontal]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    horizontal = !horizontal;
  }

  glViewport(0, 0, window_width, window_height);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_DEPTH_BUFFER_BIT);
  glUseProgram(final_shader_);
  glBindVertexArray(quad_vao_);
  glBindTextureUnit(7, composite_cbos_[0]);
  glUniform1i(glGetUniformLocation(final_shader_, "scene"), 7);
  glBindTextureUnit(8, pingpong_textures_[horizontal]);
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

  ui_graphics_.render();
}

void Renderer::shadow_map() {
  double near = near_plane;
  std::array<glm::mat4, num_cascades> light_space_matrices;
  for (int i = 0; i < num_cascades; ++i) {
    double far = near * view_plane_depth_ratio;
    const auto proj = glm::perspective(
      fov,
      aspect_ratio,
      near,
      far);
    near = far;

    const auto corners = RenderUtils::get_frustum_corners_world_space(proj, view_);
    glm::vec3 center = glm::vec3(0, 0, 0);
    for (auto& v : corners) {
      //std::cout<<glm::to_string(v)<<std::endl;
      center += glm::vec3(v);
    }
    center /= corners.size();
    auto& sun_dir = sky_.get_sun_dir();
    const auto sun_view = glm::lookAt(
      center + sun_dir,
      center,
      glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (auto& v : corners) {
      const auto trf = sun_view * v;
      minX = std::min(minX, trf.x);
      maxX = std::max(maxX, trf.x);
      minY = std::min(minY, trf.y);
      maxY = std::max(maxY, trf.y);
      minZ = std::min(minZ, trf.z);
      maxZ = std::max(maxZ, trf.z);
    }
    constexpr float zMult = 10.0f;
    if (minZ < 0)
      minZ *= zMult;
    else
      minZ /= zMult;
    if (maxZ < 0)
      maxZ /= zMult;
    else
      maxZ *= zMult;
    //std::cout<<minX<<","<<maxX<<","<<minY<<","<<maxY<<","<<minZ<<","<<maxZ<<std::endl;
    const glm::mat4 sun_proj = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    light_space_matrices[i] = sun_proj *sun_view;
    const auto tmp = light_space_matrices[i] * glm::vec4(325.f,1.f,-35.f, 1.f);
    //std::cout<<glm::to_string(tmp)<<std::endl;
    //std::cout<<i<<": "<<glm::to_string(light_space_matrices[i])<<std::endl;
  }

  glNamedBufferSubData(light_space_matrices_ubo_, 0, sizeof(glm::mat4) * num_cascades, light_space_matrices.data());
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, shadow_texture_, 0);
  glViewport(0, 0, shadow_res, shadow_res);
  glClear(GL_DEPTH_BUFFER_BIT);
  glCullFace(GL_FRONT);
  terrain_.shadow_map(*this);
}

const glm::mat4& Renderer::get_projection_matrix() const {
  return projection_;
}

const glm::mat4& Renderer::get_view_matrix() const {
  return view_;
}

void Renderer::set_highlight(const Int3D& highlight) {
  voxel_highlight_position_ = glm::dvec3(highlight[0], highlight[1], highlight[2]) - camera_offset_;
}

const Sky& Renderer::get_sky() const {
  return sky_;
}

const glm::vec3 Renderer::get_camera_world_position() const {
  return camera_world_position_;
}

const UIGraphics& Renderer::get_ui_graphics() const {
  return ui_graphics_;
}

GLuint Renderer::get_shadow_texture() const {
  return shadow_texture_;
}