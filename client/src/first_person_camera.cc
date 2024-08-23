#include "first_person_camera.h"
#include <iostream>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

FirstPersonCamera::FirstPersonCamera() {
  update_orientation();
}

void FirstPersonCamera::pan(float xoff, float yoff) {
  yaw_ += xoff * pan_sensitivity_;
  pitch_ += yoff * pan_sensitivity_;

  if (pitch_ > 89.0f)
    pitch_ = 89.0f;
  if (pitch_ < -89.0f)
    pitch_ = -89.0f;

  update_orientation();
}

void FirstPersonCamera::move_forward() {
  position_ += front_ * translation_speed_;
}
void FirstPersonCamera::move_backward() {
  position_ -= front_ * translation_speed_;
}

void FirstPersonCamera::move_up() {
  position_ += glm::vec3(0, 1, 0) * translation_speed_;
}
void FirstPersonCamera::move_down() {
  position_ -= glm::vec3(0, 1, 0) * translation_speed_;
}

void FirstPersonCamera::move_left() {
  glm::vec3 left = glm::normalize(glm::cross(front_, glm::dvec3(0, 1, 0)));
  position_ += translation_speed_ * left;
}

void FirstPersonCamera::move_right() {
  glm::vec3 right = glm::normalize(glm::cross(glm::dvec3(0, 1, 0), front_));
  position_ += translation_speed_ * right;
}
void FirstPersonCamera::turn_left() {
  yaw_ -= rotation_speed_;
  update_orientation();
}
void FirstPersonCamera::turn_right() {
  yaw_ += rotation_speed_;
  update_orientation();
}