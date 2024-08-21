#ifndef BUILD_CAMERA_H
#define BUILD_CAMERA_H

#include <array>
#include <glm/glm.hpp>
#include "camera.h"

class BuildCamera : public Camera {
public:
  BuildCamera();
  void move_forward();
  void move_backward();
  void move_left();
  void move_right();
  void move_up();
  void move_down();

private:
};

#endif