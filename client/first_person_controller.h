#ifndef FIRST_PERSON_CONTROLLER
#define FIRST_PERSON_CONTROLLER

#include "sim.h"
#include "user_controller.h"

class FirstPersonController : public UserController {
public:
  FirstPersonController(Sim& sim);
  void move_camera() override;
  bool process_input(const InputEvent& event) override;
  void init() override;
  void end() override;
};

#endif