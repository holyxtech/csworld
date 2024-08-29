#include "build_camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

BuildCamera::BuildCamera() {
  base_translation_speed_ = .5;
  yaw_ = 33.3;
  pitch_ = -33.3;
  distance_from_rotation_center_ = 50.;
  rotate_sensitivity_ = 0.1;
  update_orientation();
}

void BuildCamera::rotate(double xoff, double yoff) {
  yaw_ += xoff * rotate_sensitivity_;
  pitch_ += yoff * rotate_sensitivity_;

  if (pitch_ > 89.0f)
    pitch_ = 89.0f;
  if (pitch_ < -89.0f)
    pitch_ = -89.0f;

  update_orientation();
}

void BuildCamera::move_forward() {
  position_ += translation_speed_ * glm::dvec3(front_.x, 0, front_.z);
}
void BuildCamera::move_backward() {
  position_ -= translation_speed_ * glm::dvec3(front_.x, 0, front_.z);
}
void BuildCamera::move_left() {
  glm::dvec3 left = glm::normalize(glm::cross(front_, glm::dvec3(0, 1, 0)));
  position_ += translation_speed_ * left;
}
void BuildCamera::move_right() {
  glm::dvec3 right = glm::normalize(glm::cross(glm::dvec3(0, 1, 0), front_));
  position_ += translation_speed_ * right;
}

void BuildCamera::move_up() {
  position_ += translation_speed_ * glm::dvec3(0, 1, 0);
}
void BuildCamera::move_down() {
  position_ -= translation_speed_ * glm::dvec3(0, 1, 0);
}