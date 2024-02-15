#include "camera.h"
#include <iostream>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

Camera::Camera() {
  update_orientation();
}

const glm::vec3& Camera::get_position() const {
  return position_;
}

void Camera::set_position(glm::vec3 position) {
  position_ = position;
}

glm::mat4 Camera::get_view() const {
  return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::pan(float xoff, float yoff) {
  yaw_ += xoff * pan_sensitivity_;
  pitch_ += yoff * pan_sensitivity_;

  if (pitch_ > 89.0f)
    pitch_ = 89.0f;
  if (pitch_ < -89.0f)
    pitch_ = -89.0f;

  update_orientation();
}

void Camera::move_forward() {
  position_ += front_ * translation_speed_;
}
void Camera::move_backward() {
  position_ -= front_ * translation_speed_;
}

void Camera::move_up() {
  position_ += glm::vec3(0, 1, 0) * translation_speed_;
}
void Camera::move_down() {
  position_ -= glm::vec3(0, 1, 0) * translation_speed_;
}

void Camera::move_left() {
  glm::vec3 right = glm::normalize(glm::cross(front_, glm::vec3(0, 1, 0)));
  position_ -= right * translation_speed_;
}

void Camera::move_right() {
  glm::vec3 right = glm::normalize(glm::cross(front_, glm::vec3(0, 1, 0)));
  position_ += right * translation_speed_;
}
void Camera::turn_left() {
  yaw_ -= rotation_speed_;
  update_orientation();
}
void Camera::turn_right() {
  yaw_ += rotation_speed_;
  update_orientation();
}

void Camera::update_orientation() {
  front_.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front_.y = sin(glm::radians(pitch_));
  front_.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front_ = glm::normalize(front_);

  glm::vec3 right = glm::normalize(glm::cross(front_, glm::vec3(0, 1, 0)));
  up_ = glm::normalize(glm::cross(right, front_));
}

void Camera::scale_translation_speed(float scale) {
  translation_speed_ = base_translation_speed_ * scale;
}
void Camera::scale_rotation_speed(float scale) {
  rotation_speed_ = base_rotation_speed_ * scale;
}