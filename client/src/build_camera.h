#ifndef BUILD_CAMERA_H
#define BUILD_CAMERA_H

#include <array>
#include <glm/glm.hpp>
#include "camera.h"

class BuildCamera : public Camera {
public:
  BuildCamera();
  void rotate(double xoff, double yoff);
  void move_forward();
  void move_backward();
  void move_left();
  void move_right();
  void move_up();
  void move_down();

private:
  double distance_from_rotation_center_;
  double rotate_sensitivity_;
};

#endif