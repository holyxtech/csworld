#ifndef BUILD_CONTROLLER_H
#define BUILD_CONTROLLER_H

#include "sim.h"
#include "user_controller.h"

class BuildController : public UserController {
public:
  BuildController(Sim& sim);
  void move_camera() override;
  bool process_input(const InputEvent& event) override;
  void init() override;
  void end() override;
};

#endif