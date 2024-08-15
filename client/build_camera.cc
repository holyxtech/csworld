#include "build_camera.h"

BuildCamera::BuildCamera() {
  base_translation_speed_ = .7;
  yaw_ = 33.3;
  pitch_ = -33.3;
  update_orientation();
}

void BuildCamera::move_forward() {
  position_ += translation_speed_ * glm::dvec3(front_.x, 0, front_.z);
}
void BuildCamera::move_backward() {
  position_ -= translation_speed_ * glm::dvec3(front_.x, 0, front_.z);
}
void BuildCamera::move_left() {
  glm::dvec3 left = glm::normalize(glm::cross(front_, glm::vec3(0, 1, 0)));
  position_ += translation_speed_ * left;
}
void BuildCamera::move_right() {
  glm::dvec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), front_));
  position_ += translation_speed_ * right;
}

void BuildCamera::move_up() {
  position_ += translation_speed_ * glm::dvec3(0, 1, 0);
}
void BuildCamera::move_down() {
  position_ -= translation_speed_ * glm::dvec3(0, 1, 0);
}