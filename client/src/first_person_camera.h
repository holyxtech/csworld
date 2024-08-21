#ifndef FIRST_PERSON_CAMERA_H
#define FIRST_PERSON_CAMERA_H

#include <array>
#include <glm/glm.hpp>
#include "camera.h"

class FirstPersonCamera : public Camera {
public:
  FirstPersonCamera();
  void pan(float xoff, float yoff);
  void move_forward();
  void move_backward();
  void move_up();
  void move_down();
  void move_left();
  void move_right();
  void turn_left();
  void turn_right();

private:
  float pan_sensitivity_ = 0.1;
};

#endif