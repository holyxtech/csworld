#ifndef FIRST_PERSON_CONTROLLER
#define FIRST_PERSON_CONTROLLER

#include "sim.h"
#include "user_controller.h"

class FirstPersonController : public UserController {
public:
  FirstPersonController(Sim& sim) : UserController(sim) {}
  void move_camera() override;
  void process_inputs() override;
};

#endif