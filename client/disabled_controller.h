#ifndef DISABLED_CONTROLLER_H
#define DISABLED_CONTROLLER_H

#include <memory>
#include "sim.h"
#include "user_controller.h"

class DisabledController : public UserController {
public:
  DisabledController(Sim& sim, std::unique_ptr<UserController> previous_controller)
      : UserController(sim),
        previous_controller_(std::move(previous_controller)) {}
  void move_camera() override;
  void process_inputs() override;

private:
  std::unique_ptr<UserController> previous_controller_;
};

#endif