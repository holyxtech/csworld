#include "sky.h"
#include <string>
#include <numbers>
#include "options.h"
#include "render_utils.h"
#include "renderer.h"
#include "stb_image.h"

glm::vec3 Sky::spherical_to_cartesian(float r, float theta, float phi) {
  return glm::vec3(r * sin(theta) * sin(phi), r * cos(theta), -r * sin(theta) * cos(phi));
}

Sky::Sky() {
  RenderUtils::create_shader(&shader_, Options::instance()->getShaderPath("sky.vs"), Options::instance()->getShaderPath("sky.fs"));
  RenderUtils::create_shader(&cb_shader_, Options::instance()->getShaderPath("cb.vs"), Options::instance()->getShaderPath("cb.fs"));

  glGenTextures(1, &cube_texture_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture_);
  glUseProgram(shader_);
  glUniform1i(glGetUniformLocation(shader_, "skybox"), 1);

  std::vector<std::string> textures_faces = {
    "sky/pos_x.png",
    "sky/neg_x.png",
    "sky/pos_y.png",
    "sky/neg_y.png",
    "sky/pos_z.png",
    "sky/neg_z.png"};
  int width, height, channels;
  unsigned char* data;
  for (int i = 0; i < textures_faces.size(); ++i) {
    const std::string& texFile = Options::instance()->getImagePath(textures_faces[i]);

    data = stbi_load(texFile.c_str(), &width, &height, &channels, 0);
    glTexImage2D(
      GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
      0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  float vertices[] = {
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f};

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);

  {
    std::vector<CBVertex> mesh;
    float theta = std::numbers::pi / 2 - 0.0043633;
    float phi = std::numbers::pi / 2 - 0.0043633;
    /* float theta = std::numbers::pi / 2 - 0.01;
    float phi = std::numbers::pi / 2 - 0.01; */

    mesh.emplace_back(CBVertex{spherical_to_cartesian(1, theta, std::numbers::pi - phi), QuadCoord::tl});
    mesh.emplace_back(CBVertex{spherical_to_cartesian(1, theta, phi), QuadCoord::tr});
    mesh.emplace_back(CBVertex{spherical_to_cartesian(1, std::numbers::pi - theta, phi), QuadCoord::br});
    mesh.emplace_back(CBVertex{spherical_to_cartesian(1, theta, std::numbers::pi - phi), QuadCoord::tl});
    mesh.emplace_back(CBVertex{spherical_to_cartesian(1, std::numbers::pi - theta, phi), QuadCoord::br});
    mesh.emplace_back(CBVertex{spherical_to_cartesian(1, std::numbers::pi - theta, std::numbers::pi - phi), QuadCoord::bl});

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
    stbi_image_free(image_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
}

void Sky::render(const Renderer& renderer) const {
  glDepthFunc(GL_LEQUAL);

  auto& p = renderer.get_projection_matrix();
  auto& v = renderer.get_view_matrix();
  auto transform = p * glm::mat4(glm::mat3(v));

  {
    glUseProgram(shader_);
    auto transform_loc = glGetUniformLocation(shader_, "uTransform");
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  {
    glUseProgram(cb_shader_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sun_texture_);
    GLint texture_loc = glGetUniformLocation(cb_shader_, "cbTexture");
    glUniform1i(texture_loc, 0);
    auto transform_loc = glGetUniformLocation(cb_shader_, "uTransform");
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
    glBindVertexArray(cb_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glDepthFunc(GL_LESS);
}

GLuint Sky::get_texture() const {
  return cube_texture_;
}
