#include "build_camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

BuildCamera::BuildCamera() {
  base_translation_speed_ = .5;
  yaw_ = 33.3;
  pitch_ = -33.3;
  distance_from_rotation_center_ = 15.;
  rotate_sensitivity_ = 0.1;
  update_orientation();
}

void BuildCamera::rotate(double xoff, double yoff) {
  glm::dvec3 target = position_ + distance_from_rotation_center_ * front_;

  double angleX = xoff * rotate_sensitivity_;
  double angleY = yoff * rotate_sensitivity_;

  yaw_ += angleX;  
  pitch_ -= angleY;
  if (pitch_ > 89. || pitch_ < -89.)
    angleY = 0.;
  pitch_ = glm::clamp(pitch_, -89.0, 89.0);
  

  glm::dvec3 right = glm::normalize(glm::cross(up_, front_));

  glm::dquat qPitch = glm::angleAxis(glm::radians(-angleY), right);
  glm::dquat qYaw = glm::angleAxis(glm::radians(angleX), up_);

  glm::dquat rotation = qYaw * qPitch;

  glm::dvec3 direction = position_ - target;
  direction = rotation * direction;

  position_ = target + direction;
  front_ = glm::normalize(target - position_);
  up_ = glm::normalize(rotation * up_);

  right = glm::normalize(glm::cross(glm::dvec3(0.0, 1.0, 0.0), front_));

  up_ = glm::cross(front_, right);
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