#include "sky.h"
#include <numbers>
#include <random>
#include <string>
#include <cyPoint.h>
#include <cySampleElim.h>
#include "options.h"
#include "render_utils.h"
#include "renderer.h"
#include "stb_image.h"

glm::ivec2 Sky::transmittance_lut_size = glm::ivec2(1024, 256);
glm::ivec2 Sky::multiscattering_lut_size = {256, 256};
glm::ivec2 Sky::sky_lut_size = {512, 256};
glm::vec3 Sky::sun_dir = glm::vec3(-0.457439, 0.860318, -0.224951);
glm::vec3 Sky::sun_radiance = glm::vec3(.08f, .3f, 1.f);

namespace {
  glm::vec3 spherical_to_cartesian(float r, float theta, float phi) {
    return glm::vec3(r * sin(theta) * sin(phi), r * cos(theta), -r * sin(theta) * cos(phi));
  }
  glm::vec3 getGroupSize(int x, int y = 1, int z = 1) {
    constexpr int group_thread_size_x = 16;
    constexpr int group_thread_size_y = 16;
    constexpr int group_thread_size_z = 16;
    const int group_size_x = (x + group_thread_size_x - 1) / group_thread_size_x;
    const int group_size_y = (y + group_thread_size_y - 1) / group_thread_size_y;
    const int group_size_z = (z + group_thread_size_z - 1) / group_thread_size_z;
    return {group_size_x, group_size_y, group_size_z};
  }
  std::vector<glm::vec2> getPoissonDiskSamples(int count) {
    std::default_random_engine rng{std::random_device()()};
    std::uniform_real_distribution<float> dis(0, 1);

    std::vector<cy::Point2f> rawPoints;
    for (int i = 0; i < count * 10; ++i) {
      const float u = dis(rng);
      const float v = dis(rng);
      rawPoints.push_back({u, v});
    }

    std::vector<cy::Point2f> outputPoints(count);

    cy::WeightedSampleElimination<cy::Point2f, float, 2> wse;
    wse.SetTiling(true);
    wse.Eliminate(
      rawPoints.data(), rawPoints.size(),
      outputPoints.data(), outputPoints.size());

    std::vector<glm::vec2> result;
    for (auto& p : outputPoints)
      result.push_back({p.x, p.y});
  
    return result;
  }

} // namespace

Sky::Sky() {
  cb_shader_ = RenderUtils::create_shader("cb.vs", "cb.fs");
  set_sun_dir(52,42);

  AtmosphereProperties preset_ap;
  AtmosphereProperties std_ap = preset_ap.toStdUnit();
  transmittance_shader_ = RenderUtils::create_shader("transmittance.comp");
  glCreateBuffers(1, &atmosphere_ubo_);
  glNamedBufferStorage(atmosphere_ubo_, sizeof(AtmosphereProperties), &std_ap, GL_DYNAMIC_STORAGE_BIT);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, atmosphere_ubo_);
  glCreateTextures(GL_TEXTURE_2D, 1, &transmittance_lut_);
  glTextureStorage2D(transmittance_lut_, 1, GL_RGBA32F, transmittance_lut_size.x, transmittance_lut_size.y);
  glUseProgram(transmittance_shader_);
  glBindImageTexture(0, transmittance_lut_, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
  auto group_size = getGroupSize(transmittance_lut_size.x, transmittance_lut_size.y);
  glDispatchCompute(group_size.x, group_size.y, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  // scattering
  multiscattering_shader_ = RenderUtils::create_shader("multiscattering.comp");
  MultiscatteringParams params;
  glCreateBuffers(1, &multiscattering_params_ubo_);
  glNamedBufferStorage(multiscattering_params_ubo_, sizeof(MultiscatteringParams), &params, GL_DYNAMIC_STORAGE_BIT);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, atmosphere_ubo_);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, multiscattering_params_ubo_);
  glCreateSamplers(1, &multiscattering_sampler_);
  glSamplerParameteri(multiscattering_sampler_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(multiscattering_sampler_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(multiscattering_sampler_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(multiscattering_sampler_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(multiscattering_sampler_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glCreateBuffers(1, &multiscattering_dir_samples_ssbo_);
  auto dir_samples_arr = getPoissonDiskSamples(params.dir_sample_count);
  glNamedBufferStorage(multiscattering_dir_samples_ssbo_, params.dir_sample_count * sizeof(glm::vec2), dir_samples_arr.data(), GL_DYNAMIC_STORAGE_BIT);
  glCreateTextures(GL_TEXTURE_2D, 1, &multiscattering_lut_);
  glTextureStorage2D(multiscattering_lut_, 1, GL_RGBA32F, multiscattering_lut_size.x, multiscattering_lut_size.y);
  glBindImageTexture(0, multiscattering_lut_, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, multiscattering_dir_samples_ssbo_);
  glBindTextureUnit(0, transmittance_lut_);
  glBindSampler(0, multiscattering_sampler_);
  glUseProgram(multiscattering_shader_);
  group_size = getGroupSize(multiscattering_lut_size.x, multiscattering_lut_size.y);
  glDispatchCompute(group_size.x, group_size.y, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  // sky lut
  sky_lut_shader_ = RenderUtils::create_shader(
    "sky_quad.vs",
    "sky_lut.fs");
  glCreateFramebuffers(1, &sky_lut_fbo_);
  glCreateRenderbuffers(1, &sky_lut_rbo_);
  glNamedRenderbufferStorage(sky_lut_rbo_, GL_DEPTH24_STENCIL8, sky_lut_size.x, sky_lut_size.y);
  glNamedFramebufferRenderbuffer(sky_lut_fbo_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sky_lut_rbo_);
  glCreateTextures(GL_TEXTURE_2D, 1, &sky_lut_);
  glTextureStorage2D(sky_lut_, 1, GL_RGBA32F, sky_lut_size.x, sky_lut_size.y);
  glNamedFramebufferTexture(sky_lut_fbo_, GL_COLOR_ATTACHMENT0, sky_lut_, 0);
  glCreateBuffers(1, &sky_lut_params_ubo_);
  glNamedBufferData(sky_lut_params_ubo_, sizeof(SkyLutParams), nullptr, GL_DYNAMIC_DRAW);
  glCreateVertexArrays(1, &sky_lut_vao_);

  // sky
  sky_shader_ = RenderUtils::create_shader(
    "sky_quad.vs",
    "sky.fs");
  glCreateVertexArrays(1, &sky_vao_);
  glCreateBuffers(1, &sky_params_ubo_);
  glNamedBufferData(sky_params_ubo_, sizeof(SkyParams), nullptr, GL_DYNAMIC_DRAW);
  glUseProgram(sky_shader_);
  float ratio = (Renderer::window_width) / static_cast<float>(Renderer::window_height);
  glUniform1fv(glGetUniformLocation(sky_shader_, "WOverH"), 1, &ratio);

  std::vector<CBVertex> mesh;
  float theta = std::numbers::pi / 2 - 0.0043633;
  float phi = std::numbers::pi / 2 - 0.0043633;
  /* float theta = std::numbers::pi / 2 - 0.01;
  float phi = std::numbers::pi / 2 - 0.01; */

  mesh.emplace_back(spherical_to_cartesian(1, theta, std::numbers::pi - phi), QuadCoord::tl);
  mesh.emplace_back(spherical_to_cartesian(1, theta, phi), QuadCoord::tr);
  mesh.emplace_back(spherical_to_cartesian(1, std::numbers::pi - theta, phi), QuadCoord::br);
  mesh.emplace_back(spherical_to_cartesian(1, theta, std::numbers::pi - phi), QuadCoord::tl);
  mesh.emplace_back(spherical_to_cartesian(1, std::numbers::pi - theta, phi), QuadCoord::br);
  mesh.emplace_back(spherical_to_cartesian(1, std::numbers::pi - theta, std::numbers::pi - phi), QuadCoord::bl);

  glGenVertexArrays(1, &cb_vao_);
  glGenBuffers(1, &cb_vbo_);
  glBindVertexArray(cb_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, cb_vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(CBVertex) * mesh.size(), mesh.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CBVertex), (void*)offsetof(CBVertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CBVertex), (void*)offsetof(CBVertex, uvs));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glGenTextures(1, &sun_texture_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sun_texture_);
  int width, height, channels;
  const std::string& path = Options::instance()->getImagePath("sun.png");
  auto* image_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  stbi_image_free(image_data);
}

void Sky::generate_sky_lut(const Renderer& renderer) const {
  glDepthFunc(GL_LEQUAL);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, atmosphere_ubo_);

  SkyLutParams params;
  auto camera_position = renderer.get_camera().get_position();
  params.sun_dir = -sun_dir;
  params.sun_radiance = sun_radiance;
  params.view_pos = renderer.get_camera_offset_position();
  params.view_pos.y = std::max(static_cast<float>(camera_position.y), 1.f);
  params.ray_march_step_count = 40;
  params.enable_multi_scattering = 1;
  glNamedBufferSubData(sky_lut_params_ubo_, 0, sizeof(SkyLutParams), &params);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, sky_lut_params_ubo_);
  glUseProgram(sky_lut_shader_);
  glBindTextureUnit(0, transmittance_lut_);
  glBindTextureUnit(1, multiscattering_lut_);
  glBindFramebuffer(GL_FRAMEBUFFER, sky_lut_fbo_);
  glViewport(0, 0, sky_lut_size.x, sky_lut_size.y);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(sky_lut_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDepthFunc(GL_LESS);
}

void Sky::render(const Renderer& renderer) const {
  glDepthFunc(GL_LEQUAL);

  // sky
  SkyParams params;
  auto& camera = renderer.get_camera();
  auto& front = camera.get_front();
  auto& up = camera.get_up();
  params.camera_dir = front;
  params.up = up;
  params.right = glm::normalize(glm::cross(up, front));
  params.exposure = 10.f;
  params.scale = std::tan(0.5f * renderer.fov);
  glNamedBufferSubData(sky_params_ubo_, 0, sizeof(SkyParams), &params);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, sky_params_ubo_);
  glUseProgram(sky_shader_);
  glBindTextureUnit(0, sky_lut_);
  glBindVertexArray(sky_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // sun
  glUseProgram(cb_shader_);
  glBindTextureUnit(0, sun_texture_);
  GLint texture_loc = glGetUniformLocation(cb_shader_, "cbTexture");
  glUniform1i(texture_loc, 0);
  auto transform_loc = glGetUniformLocation(cb_shader_, "uTransform");
  glm::vec3 originalDirection = glm::vec3(1, 0, 0);
  glm::vec3 targetDirection = glm::normalize(sun_dir);
  glm::vec3 rotationAxis = glm::cross(originalDirection, targetDirection);
  float angle = acos(glm::dot(originalDirection, targetDirection));
  glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, rotationAxis);
  auto& p = renderer.get_projection_matrix();
  auto& v = renderer.get_view_matrix();
  auto transform = p * glm::mat4(glm::mat3(v)) * rotationMatrix;
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
  glBindVertexArray(cb_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDepthFunc(GL_LESS);
}

const glm::vec3& Sky::get_sun_dir() const {
  return sun_dir;
}

void Sky::set_sun_dir(const glm::vec3& dir) {
  sun_dir = dir;
}

void Sky::set_sun_dir(float azimuth, float polar) {
  // Convert angles from degrees to radians
  float azimuth_rads = glm::radians(azimuth); // Azimuth angle (0 to 360 degrees)
  float polar_rads = glm::radians(polar);     // Polar angle (0 to 180 degrees)

  // Ensure polar is within the valid range [0, 180] degrees
  polar_rads = glm::clamp(polar_rads, 0.0f, glm::pi<float>());

  // Calculate Cartesian coordinates based on standard spherical coordinate conventions
  float x = cos(polar_rads) * sin(azimuth_rads);
  float y = sin(polar_rads) * sin(azimuth_rads);
  float z = cos(azimuth_rads);

  // Set the sun direction vector
  sun_dir = glm::vec3(x, y, z);
}