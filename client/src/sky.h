#ifndef SKY_H
#define SKY_H

#include <vector>
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

class Renderer;

class Sky {
public:
  Sky();
  void render(const Renderer& renderer) const;
  void generate_sky_lut(const Renderer& renderer) const;

  GLuint get_texture() const;
  const glm::vec3& get_sun_dir() const;
  void set_sun_dir(const glm::vec3& dir);

private:
  struct AtmosphereProperties {
    glm::vec3 rayleigh_scattering = {5.802f, 13.558f, 33.1f}; // 10^(-6)m^(-1)
    float rayleigh_density_h = 8.f;                           // km

    float mie_scattering = 3.996f;
    float mie_asymmetry_g = 0.88f; // scalar
    float mie_absorption = 4.4f;
    float mie_density_h = 1.2f;

    glm::vec3 ozone_absorption = {0.65f, 1.881f, 0.085f}; // 10^(-6)m^(-1)
    // absolute height = ground_radius + ozone_center_h
    float ozone_center_h = 25; // km
    float ozone_width = 30;    // km

    float ground_radius = 6360;         // km
    float top_atmosphere_radius = 6460; // km
    float padding = 0;

    // to meter
    AtmosphereProperties toStdUnit() const {
      AtmosphereProperties ret = *this;

      ret.rayleigh_scattering *= 1e-6f;
      ret.rayleigh_density_h *= 1e3f;

      ret.mie_scattering *= 1e-6f;
      ret.mie_absorption *= 1e-6f;
      ret.mie_density_h *= 1e3f;

      ret.ozone_absorption *= 1e-6f;
      ret.ozone_center_h *= 1e3f;
      ret.ozone_width *= 1e3f;

      ret.ground_radius *= 1e3f;
      ret.top_atmosphere_radius *= 1e3f;

      return ret;
    }
  };

  struct CBVertex {
    glm::vec3 position;
    glm::vec2 uvs;
  };

  // sky and atmosphere
  GLuint transmittance_shader_;
  GLuint atmosphere_ubo_;
  GLuint transmittance_lut_;
  static glm::ivec2 transmittance_lut_size;

  struct MultiscatteringParams {
    glm::vec3 ground_albedo = glm::vec3(0.3f);
    int dir_sample_count = 64;
    glm::vec3 sun_intensity = glm::vec3(1);
    int ray_march_step_count = 256;
  } multiscattering_params_;
  GLuint multiscattering_shader_;
  GLuint multiscattering_params_ubo_;
  GLuint multiscattering_lut_;
  GLuint multiscattering_dir_samples_ssbo_;
  GLuint multiscattering_sampler_;
  static glm::ivec2 multiscattering_lut_size;

  struct SkyLutParams {
    glm::vec3 view_pos;
    int ray_march_step_count;
    glm::vec3 sun_dir;
    int enable_multi_scattering;
    glm::vec3 sun_radiance;
    float pad = 0.f;
  };
  GLuint sky_lut_shader_;
  GLuint sky_lut_params_ubo_;
  GLuint sky_lut_fbo_;
  GLuint sky_lut_rbo_;
  static glm::ivec2 sky_lut_size;
  GLuint sky_lut_;
  GLuint sky_lut_vao_;

  struct SkyParams {
    glm::vec3 camera_dir;
    float exposure;
    glm::vec3 up;
    float scale; // tan(fov/2)
    glm::vec3 right;
    float pad;
  } sky_params_;
  GLuint sky_shader_;
  GLuint sky_vao_;
  GLuint sky_params_ubo_;

  // celestial body
  GLuint cb_shader_;
  GLuint cb_vao_;
  GLuint cb_vbo_;
  GLuint sun_texture_;

  static glm::vec3 sun_dir;
  static glm::vec3 sun_radiance;
};

#endif