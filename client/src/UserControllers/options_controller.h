#ifndef OPTIONS_CONTROLLER_H
#define OPTIONS_CONTROLLER_H

#include "sim.h"
#include "user_controller.h"
#include <memory>

class OptionsController : public UserController {
public:
  OptionsController(Sim& sim, std::unique_ptr<UserController> previous_controller);
  void init() override;
  void end() override;
  void process_input(const InputEvent& event) override;

private:
  std::unique_ptr<UserController> previous_controller_;
};

#endif