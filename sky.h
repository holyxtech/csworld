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
  GLuint get_texture() const;
  const glm::vec3& get_sun_dir() const;
  void set_sun_dir(const glm::vec3& dir);

private:
  struct CBVertex {
    glm::vec3 position;
    glm::vec2 uvs;
  };

  GLuint shader_;
  GLuint vao_;
  GLuint vbo_;
  GLuint cube_texture_;

  // celestial body
  GLuint cb_shader_;
  GLuint cb_vao_;
  GLuint cb_vbo_;
  GLuint sun_texture_;

  glm::vec3 sun_dir_ = glm::vec3(1.f,1.f,0.f);
};

#endif