#include "sky.h"
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "render_utils.h"
#include "renderer.h"
#include "stb_image.h"

Sky::Sky() {
  glGenTextures(1, &cube_texture_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture_);
  glUniform1i(glGetUniformLocation(shader_, "skybox"), 0);

  std::vector<std::string> textures_faces = {
    "images/sky/pos_x.png",
    "images/sky/neg_x.png",
    "images/sky/pos_y.png",
    "images/sky/neg_y.png",
    "images/sky/pos_z.png",
    "images/sky/neg_z.png"};
  int width, height, channels;
  unsigned char* data;
  for (int i = 0; i < textures_faces.size(); ++i) {
    data = stbi_load(textures_faces[i].c_str(), &width, &height, &channels, 0);
    glTexImage2D(
      GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
      0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  RenderUtils::create_shader(&shader_, "shaders/sky_vertex.glsl", "shaders/sky_fragment.glsl");

  float vertices[] = {
    // positions
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
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Sky::render(const Renderer& renderer) const {
  glDepthFunc(GL_LEQUAL);
  glUseProgram(shader_);
  auto& p = renderer.get_projection_matrix();
  auto& v = renderer.get_view_matrix();
  auto transform = p * glm::mat4(glm::mat3(v));
  auto transform_loc = glGetUniformLocation(shader_, "uTransform");
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &transform[0][0]);
  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthFunc(GL_LESS);
}