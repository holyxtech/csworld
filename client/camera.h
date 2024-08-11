#ifndef CAMERA_H
#define CAMERA_H

#include <array>
#include <glm/glm.hpp>

class Camera {
public:
  Camera();
  void print() const;
  void set_orientation(double yaw, double pitch);
  double get_yaw() const;
  double get_pitch() const;

  glm::mat4 get_view(glm::dvec3 camera_offset) const;
  const glm::dvec3& get_position() const;
  const glm::vec3& get_front() const;
  const glm::vec3& get_up() const;
  glm::vec3 get_world_position(glm::dvec3 camera_offset) const;
  void set_position(glm::dvec3 position);
  void scale_translation_speed(double scale);
  void scale_rotation_speed(double scale);
  void set_base_translation_speed(double base_translation_speed);

protected:
  void update_orientation();

  glm::dvec3 position_;
  glm::vec3 front_;
  glm::vec3 up_;

  double yaw_ = 0.f;
  double pitch_ = 0.f;

  double base_translation_speed_ = 0.2;
  double base_rotation_speed_ = 1;
  double translation_speed_ = 0.1;
  double rotation_speed_ = 1;
};

#endif