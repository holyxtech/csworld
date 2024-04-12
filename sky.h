#ifndef SKY_H
#define SKY_H

#define GLM_FORCE_LEFT_HANDED
#include <vector>
#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

class Renderer;

class Sky {
public:
  Sky();
  void render(const Renderer& renderer) const;
  GLuint get_texture() const;

private:
  struct CBVertex {
    glm::vec3 position;
    glm::vec2 uvs;
  };

  static glm::vec3 spherical_to_cartesian(float r, float theta, float phi);

  GLuint shader_;
  GLuint vao_;
  GLuint vbo_;
  GLuint cube_texture_;

  // celestial body
  GLuint cb_shader_;
  GLuint cb_vao_;
  GLuint cb_vbo_;
};

#endif