#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

class Camera {
public:
  Camera();
  void pan(float xoff, float yoff);
  void move_forward();
  void move_backward();
  void move_up();
  void move_down();
  void move_left();
  void move_right();
  void turn_left();
  void turn_right();

  glm::mat4 get_view(glm::dvec3 camera_offset) const;
  const glm::dvec3& get_position() const;
  void set_position(glm::dvec3 position);
  void scale_translation_speed(float scale);
  void scale_rotation_speed(float scale);

private:
  void update_orientation();

  glm::dvec3 position_;
  glm::vec3 front_;
  glm::vec3 up_;

  float yaw_ = 0.f;
  float pitch_ = 0.f;

  float pan_sensitivity_ = 0.1;

  float base_translation_speed_ = 4;
  float base_rotation_speed_ = 1;
  float translation_speed_ = 0.1;
  float rotation_speed_ = 1;
};

#endif