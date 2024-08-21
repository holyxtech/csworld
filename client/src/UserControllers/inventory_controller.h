#ifndef INVENTORY_CONTROLLER_H
#define INVENTORY_CONTROLLER_H

#include "user_controller.h"
#include "sim.h"

class InventoryController : public UserController {
public:
  InventoryController(Sim& sim);
  void move_camera() override;
  void process_input(const InputEvent& event) override;
  void init() override;
  void end() override;
};

#endif