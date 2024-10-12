#include "renderer.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <utility>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "options.h"
#include "render_utils.h"
#include "sim.h"
#include "stb_image.h"
#include "types.h"

int Renderer::window_width = 2560;
int Renderer::window_height = 1440;
double Renderer::aspect_ratio = Renderer::window_width / static_cast<double>(Renderer::window_height);
double Renderer::fov = glm::radians(45.);
double Renderer::near_plane = .1;
double Renderer::far_plane = 1000.;
int Renderer::shadow_res = 2048;
GLuint Renderer::blur_texture_width = Renderer::window_width / 8;
GLuint Renderer::blur_texture_height = Renderer::window_height / 8;
std::array<float, Renderer::num_cascades> Renderer::cascade_far_planes = {40, 200, 1000};

namespace {
  float ourLerp(float a, float b, float f) {
    return a + f * (b - a);
  }
} // namespace

Renderer::Renderer(Sim& sim)
    : sim_(sim), ui_graphics_(sim) {

  // Shaders
  shaders_["VoxelHighlight"] =
    Shader(
      RenderUtils::create_shader("voxel_highlight.vs", "voxel_highlight.fs"),
      {{"MainDepth", 0}},
      {{"Common", 0}});
  shaders_["GroundSelection"] = Shader(
    RenderUtils::create_shader("ground_selection.vs", "ground_selection.fs"),
    {{"MainDepth", 0},
     {"MainColor", 1}},
    {{"Common", 0}});
  composite_shader_ = RenderUtils::create_shader("quad.vs", "composite.fs");
  blur_shader_ = RenderUtils::create_shader("blur.vs", "blur.fs");
  final_shader_ = RenderUtils::create_shader("final.vs", "final.fs");
  ssao_shader_ = RenderUtils::create_shader("ssao_quad.vs", "ssao.fs");
  ssao_blur_shader_ = RenderUtils::create_shader("ssao_quad.vs", "ssao_blur.fs");
  ssao_apply_shader_ = RenderUtils::create_shader("quad.vs", "ssao_apply.fs");

  // Generic
  projection_ = glm::perspective(fov, aspect_ratio, near_plane, far_plane);
  glCreateBuffers(1, &common_ubo_);
  common_block_.projection = projection_;
  common_block_.inv_projection = glm::inverse(projection_);
  glNamedBufferStorage(common_ubo_, sizeof(CommonBlock), &common_block_, GL_DYNAMIC_STORAGE_BIT);
  glCreateVertexArrays(1, &quad_vao_);
  glGenFramebuffers(1, &pingpong_primary_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, pingpong_primary_fbo_);
  glGenTextures(1, &pingpong_primary_cbo_);
  glBindTexture(GL_TEXTURE_2D, pingpong_primary_cbo_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpong_primary_cbo_, 0);

  // Opaque / water
  auto set_up_framebuffers =
    [](GLuint& fbo, GLuint& cbo, GLuint& dbo, std::vector<std::reference_wrapper<GLuint>> gbufs) -> void {
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbo, 0);
    for (std::size_t i = 0; i < gbufs.size(); ++i) {
      auto& gbuf = gbufs[i].get();
      glGenTextures(1, &gbuf);
      glBindTexture(GL_TEXTURE_2D, gbuf);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 + i, GL_TEXTURE_2D, gbuf, 0);
    }
    std::array<GLuint, 3> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments.data());
  };
  set_up_framebuffers(main_fbo_, main_cbo_, main_dbo_, {std::ref(main_camera_normal_)});
  set_up_framebuffers(water_fbo_, water_cbo_, water_dbo_, {std::ref(water_camera_position_), std::ref(water_camera_normal_)});
  glGenFramebuffers(1, &composite_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, composite_fbo_);
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
  std::array<GLuint, 2> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments.data());

  // Blur
  glGenFramebuffers(2, pingpong_fbos_.data());
  glGenTextures(2, pingpong_cbos_.data());
  for (std::size_t i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbos_[i]);
    glBindTexture(GL_TEXTURE_2D, pingpong_cbos_[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, blur_texture_width, blur_texture_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpong_cbos_[i], 0);
  }

  // SSR
  float sx = float(window_width) / 2.0;
  float sy = float(window_height) / 2.0;
  auto warp_to_screen_space = glm::mat4(1.0);
  warp_to_screen_space[0] = glm::vec4(sx, 0, 0, sx);
  warp_to_screen_space[1] = glm::vec4(0, sy, 0, sy);
  warp_to_screen_space = glm::transpose(warp_to_screen_space);
  glm::mat4 pixel_projection = warp_to_screen_space * projection_;
  glUseProgram(composite_shader_);
  auto pixel_projection_loc = glGetUniformLocation(composite_shader_, "pixelProjection");
  glUniformMatrix4fv(pixel_projection_loc, 1, GL_FALSE, glm::value_ptr(pixel_projection));

  // Shadow mapping
  glCreateBuffers(1, &light_space_matrices_ubo_);
  glNamedBufferStorage(light_space_matrices_ubo_, sizeof(glm::mat4) * num_cascades, nullptr, GL_DYNAMIC_STORAGE_BIT);
  glCreateBuffers(1, &shadow_block_ubo_);
  shadow_block_.far_plane = far_plane;
  shadow_block_.light_dir = sky_.get_sun_dir();
  for (int i = 0; i < num_cascades; ++i) {
    shadow_block_.cascade_plane_distances[i / 4][i % 4] = cascade_far_planes[i];
  }
  glNamedBufferStorage(shadow_block_ubo_, sizeof(ShadowBlock), &shadow_block_, GL_DYNAMIC_STORAGE_BIT);
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
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  constexpr float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_texture_, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // SSAO
  glGenFramebuffers(1, &ssao_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo_);
  glGenTextures(1, &ssao_cbo_);
  glBindTexture(GL_TEXTURE_2D, ssao_cbo_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_cbo_, 0);
  glGenFramebuffers(1, &ssao_blur_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo_);
  glGenTextures(1, &ssao_blur_cbo_);
  glBindTexture(GL_TEXTURE_2D, ssao_blur_cbo_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_cbo_, 0);
  std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
  std::default_random_engine generator;
  const int num_kernels = 8;
  std::array<glm::vec3, num_kernels> ssaoKernel;
  for (int i = 0; i < num_kernels; ++i) {
    glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
    sample = glm::normalize(sample);
    sample *= randomFloats(generator);
    float scale = float(i) / num_kernels;
    scale = ourLerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    ssaoKernel[i] = sample;
  }
  glUseProgram(ssao_shader_);
  glUniform3fv(glGetUniformLocation(ssao_shader_, "samples"), num_kernels, &ssaoKernel[0][0]);
  std::vector<glm::vec3> ssaoNoise;
  for (unsigned int i = 0; i < 16; i++) {
    glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
    noise = glm::normalize(noise);
    ssaoNoise.push_back(noise);
  }
  glGenTextures(1, &ssao_noise_texture_);
  glBindTexture(GL_TEXTURE_2D, ssao_noise_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  textures_["MainDepth"] = main_dbo_;
  textures_["MainColor"] = main_cbo_;
  textures_["ShadowDepth"] = shadow_texture_;
  uniform_buffers_["Common"] = common_ubo_;
  uniform_values_["ShadowBias"] = UniformValue{UniformType::Float, 0.0001f};

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(2.f);
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
      world_offset_ = glm::dvec3(loc[0] * Chunk::sz_x, loc[1] * Chunk::sz_y, loc[2] * Chunk::sz_z);
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
  view_ = camera.get_view(world_offset_);
  camera_offset_position_ = camera.get_position(world_offset_);
}

void Renderer::render_scene() {
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  common_block_.normal = glm::transpose(glm::inverse(view_));
  common_block_.view = view_;
  common_block_.frame = frame_;
  glNamedBufferSubData(common_ubo_, 0, sizeof(CommonBlock), &common_block_);

  shadow_map();

  shadow_block_.light_dir = sky_.get_sun_dir();
  shadow_block_.bias = std::any_cast<float>(uniform_values_["ShadowBias"].data);
  glNamedBufferSubData(shadow_block_ubo_, 0, sizeof(ShadowBlock), &shadow_block_);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, common_ubo_);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, light_space_matrices_ubo_);
  glBindBufferBase(GL_UNIFORM_BUFFER, 2, shadow_block_ubo_);
  glViewport(0, 0, window_width, window_height);
  glBindFramebuffer(GL_FRAMEBUFFER, water_fbo_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  terrain_.render_water(*this);
  glBindFramebuffer(GL_FRAMEBUFFER, main_fbo_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  terrain_.render(*this);
  glDepthFunc(GL_LEQUAL);
  glDisable(GL_BLEND);
  ssao();
  glDepthFunc(GL_LESS);
  glBindFramebuffer(GL_FRAMEBUFFER, pingpong_primary_fbo_);
  glUseProgram(ssao_apply_shader_);
  glBindTextureUnit(0, main_cbo_);
  glUniform1i(glGetUniformLocation(ssao_apply_shader_, "MainColor"), 0);
  glBindTextureUnit(1, ssao_blur_cbo_);
  glUniform1i(glGetUniformLocation(ssao_apply_shader_, "SSAO"), 1);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBlitNamedFramebuffer(
    pingpong_primary_fbo_, main_fbo_, 0, 0, window_width, window_height,
    0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glEnable(GL_BLEND);
  glBindFramebuffer(GL_FRAMEBUFFER, main_fbo_);
  terrain_.render_irregular(*this);
  glDisable(GL_BLEND);

  sky_.generate_sky_lut(*this);
  glViewport(0, 0, window_width, window_height);
  glBindFramebuffer(GL_FRAMEBUFFER, main_fbo_);
  sky_.render(*this);

  glBindBufferBase(GL_UNIFORM_BUFFER, 0, common_ubo_);
  glViewport(0, 0, window_width, window_height);
  glBindFramebuffer(GL_FRAMEBUFFER, composite_fbo_);
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

  bool horizontal = true;
  glNamedFramebufferReadBuffer(composite_fbo_, GL_COLOR_ATTACHMENT0 + 1);
  glBlitNamedFramebuffer(
    composite_fbo_, pingpong_fbos_[horizontal], 0, 0, window_width, window_height,
    0, 0, blur_texture_width, blur_texture_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glUseProgram(blur_shader_);
  glViewport(0, 0, blur_texture_width, blur_texture_height);
  glActiveTexture(GL_TEXTURE6);
  glUniform1i(glGetUniformLocation(blur_shader_, "image"), 6);
  for (int i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbos_[!horizontal]);
    glUniform1i(glGetUniformLocation(blur_shader_, "horizontal"), !horizontal);
    glBindTexture(GL_TEXTURE_2D, pingpong_cbos_[horizontal]);
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
  glBindTextureUnit(8, pingpong_cbos_[horizontal]);
  glUniform1i(glGetUniformLocation(final_shader_, "bloom"), 8);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  ++frame_;
}

void Renderer::render(const DrawCommand& command) {
  glUseProgram(command.shader_id);
  if (command.blend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);

  if (command.depth_test)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);

  for (auto& binding : command.uniform_buffer_bindings)
    glBindBufferBase(GL_UNIFORM_BUFFER, binding.binding_point, binding.id);

  for (auto& binding : command.uniforms) {
    auto binding_type = binding.type;
    auto binding_id = binding.id;
    switch (binding_type) {
    case UniformType::Matrix4f:
      glUniformMatrix4fv(binding_id, 1, GL_FALSE, (const GLfloat*)binding.data);
      break;
    }
  }

  for (auto& binding : command.texture_bindings)
    glBindTextureUnit(binding.unit, binding.id);

  GLuint vao = vbo_to_vao_[command.vertex_buffer_id];
  glBindVertexArray(vao);

  GLenum draw_mode;
  switch (command.primitive_type) {
  case PrimitiveType::Lines:
    draw_mode = GL_LINES;
    break;
  case PrimitiveType::Triangles:
    draw_mode = GL_TRIANGLES;
    break;
  }

  if (command.index_buffer_id == -1) {
    glDrawArrays(draw_mode, 0, command.vertex_count);
  } else {
    glDrawElements(draw_mode, command.index_count, GL_UNSIGNED_INT, 0);
  }
}

void Renderer::upload_buffer_data(
  std::uint32_t component_id, BufferType buffer_type, unsigned int offset, unsigned int size, const void* data) {
  GLuint buffer;
  GLenum target;
  switch (buffer_type) {
  case BufferType::Vertex:
    buffer = vertex_buffer_ids_[component_id];
    target = GL_ARRAY_BUFFER;
    break;
  case BufferType::Index:
    buffer = index_buffer_ids_[component_id];
    target = GL_ELEMENT_ARRAY_BUFFER;
    break;
  }
  glBindBuffer(target, buffer);
  glBufferSubData(target, offset, size, data);
}

std::uint32_t Renderer::register_scene_component(const SceneComponent& scene_component) {
  std::uint32_t id = next_component_id_++;
  GLuint vao;
  GLuint vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  vertex_buffer_ids_[id] = vbo;
  vbo_to_vao_[vbo] = vao;

  GLenum draw_type = scene_component.check_flag(SceneComponentFlags::Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  auto& vertices = scene_component.get_vertices();
  glBufferData(GL_ARRAY_BUFFER, sizeof(std::uint8_t) * vertices.size(), vertices.data(), draw_type);

  auto& indices = scene_component.get_indices();
  if (indices.size() > 0) {
    GLuint ebo;
    glGenBuffers(1, &ebo);
    index_buffer_ids_[id] = ebo;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), draw_type);
  }

  auto& attributes = scene_component.get_vertex_attributes();
  int attrib_id = 0;
  for (auto& attribute : attributes) {
    switch (attribute.type) {
    case VertexAttributeType::Float:
      glVertexAttribPointer(attrib_id, attribute.component_count, GL_FLOAT, GL_FALSE, sizeof(float) * attribute.component_count, (void*)0);
      break;
    case VertexAttributeType::Int:
      break;
    }
    glEnableVertexAttribArray(attrib_id);
    ++attrib_id;
  }

  return id;
}

void Renderer::ssao() {
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo_);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(ssao_shader_);

  glBindBufferBase(GL_UNIFORM_BUFFER, 0, common_ubo_);
  glBindTextureUnit(0, main_dbo_);
  glUniform1i(glGetUniformLocation(ssao_shader_, "gDepth"), 0);
  glBindTextureUnit(1, main_camera_normal_);
  glUniform1i(glGetUniformLocation(ssao_shader_, "gNormal"), 1);
  glBindTextureUnit(2, ssao_noise_texture_);
  glUniform1i(glGetUniformLocation(ssao_shader_, "texNoise"), 2);
  auto noise_scale = glm::vec2(window_width / 4.0, window_height / 4.0);
  glUniform2fv(glGetUniformLocation(ssao_shader_, "noiseScale"), 1, glm::value_ptr(noise_scale));
  glBindVertexArray(quad_vao_);

  /*   GLuint query;
    glGenQueries(1, &query);
    glBeginQuery(GL_TIME_ELAPSED, query); */
  glDrawArrays(GL_TRIANGLES, 0, 6);
  /* glEndQuery(GL_TIME_ELAPSED);
  GLuint64 elapsedTime;
  glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsedTime);
  std::cout << "Time taken: " << (elapsedTime / 1000000.0) << " ms" << std::endl;
  glDeleteQueries(1, &query); */

  glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo_);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(ssao_blur_shader_);
  glBindTextureUnit(0, ssao_cbo_);
  glUniform1i(glGetUniformLocation(ssao_blur_shader_, "ssaoInput"), 0);
  glBindVertexArray(quad_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::shadow_map() {
  double near_distance = near_plane;
  std::array<glm::mat4, num_cascades> light_space_matrices;

  for (int i = 0; i < num_cascades; ++i) {
    double far_distance = cascade_far_planes[i];
    const auto proj = glm::perspective(
      fov,
      aspect_ratio,
      near_distance,
      far_distance);
    near_distance = far_distance;

    const auto corners = RenderUtils::get_frustum_corners_world_space(proj, view_);
    glm::vec3 center = glm::vec3(0, 0, 0);
    for (auto& v : corners) {
      center += v;
    }
    center /= corners.size();
    auto& sun_dir = sky_.get_sun_dir();

    const auto sun_view = glm::lookAt(
      center + sun_dir,
      center,
      glm::vec3(0.0f, 1.0f, 0.0f));

    float radius = 0;
    for (auto& corner : corners) {
      float distance_from_center = glm::length(corner - center);
      radius = std::max(radius, distance_from_center);
    }
    radius = std::ceil(radius);

    glm::vec3 max_sphere = glm::vec3(radius);
    glm::vec3 min_sphere = -glm::vec3(radius);
    float minZ = min_sphere.z;
    float maxZ = max_sphere.z;
    constexpr float zMult = 10.0f;
    if (minZ < 0)
      minZ *= zMult;
    else
      minZ /= zMult;
    if (maxZ < 0)
      maxZ /= zMult;
    else
      maxZ *= zMult;
    glm::mat4 sun_proj = glm::ortho(min_sphere.x, max_sphere.x, min_sphere.y, max_sphere.y, minZ, maxZ);

    glm::vec4 light_space_world_origin = sun_proj * sun_view * glm::vec4(0.f, 0.f, 0.f, 1.f);
    glm::vec4 texel_space_world_origin = light_space_world_origin * shadow_res / 2.f;
    glm::vec4 rounded = glm::round(texel_space_world_origin);
    glm::vec4 rounded_difference = (rounded - texel_space_world_origin) * 2.f / shadow_res;
    rounded_difference.z = 0.f;
    rounded_difference.w = 0.f;
    sun_proj[3] += rounded_difference;

    light_space_matrices[i] = sun_proj * sun_view;
  }

  glBindBufferBase(GL_UNIFORM_BUFFER, 0, light_space_matrices_ubo_);
  glNamedBufferSubData(light_space_matrices_ubo_, 0, sizeof(glm::mat4) * num_cascades, light_space_matrices.data());
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_texture_, 0);
  glViewport(0, 0, shadow_res, shadow_res);
  glClear(GL_DEPTH_BUFFER_BIT);
  terrain_.shadow_map(*this);
}

const glm::mat4& Renderer::get_projection_matrix() const {
  return projection_;
}

const glm::mat4& Renderer::get_view_matrix() const {
  return view_;
}

Sky& Renderer::get_sky() {
  return sky_;
}

const glm::vec3& Renderer::get_camera_offset_position() const {
  return camera_offset_position_;
}

UIGraphics& Renderer::get_ui_graphics() {
  return ui_graphics_;
}

const Camera& Renderer::get_camera() const {
  return sim_.get_camera();
}

GLuint Renderer::get_texture(const std::string& name) const {
  return textures_.at(name);
}

GLuint Renderer::get_uniform_buffer(const std::string& name) const {
  return uniform_buffers_.at(name);
}

const Shader Renderer::get_shader(const std::string& name) const {
  return shaders_.at(name);
}

GLuint Renderer::get_vertex_buffer_id(std::uint32_t component_id) { return vertex_buffer_ids_[component_id]; }
GLuint Renderer::get_index_buffer_id(std::uint32_t component_id) { return index_buffer_ids_[component_id]; }
GLint Renderer::get_uniform_id(const std::string& shader_name, const std::string& uniform_name) const {
  auto& shader = shaders_.at(shader_name);
  GLuint id = shader.get_id();
  return glGetUniformLocation(id, uniform_name.c_str());
}
const glm::dvec3& Renderer::get_world_offset() const {
  return world_offset_;
}

void Renderer::set_uniform_value(const std::string& uniform_name, const UniformValue& value) {
  uniform_values_[uniform_name] = value;
}

const UniformValue& Renderer::get_uniform_value(const std::string& uniform_name) const {
  return uniform_values_.at(uniform_name);
}