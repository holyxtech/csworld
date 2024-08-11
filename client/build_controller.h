#ifndef BUILD_CONTROLLER_H
#define BUILD_CONTROLLER_H

#include "sim.h"
#include "user_controller.h"

class BuildController : public UserController {
public:
  BuildController(Sim& sim)
      : UserController(sim) {}
  void move_camera() override;
  void process_inputs() override;
};

#endif