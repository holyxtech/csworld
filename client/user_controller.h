#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include <memory>
#include "readerwriterqueue.h"
#include "input.h"

class Sim;

class UserController {
public:
  UserController(Sim& sim) : sim_(sim) {}
  virtual void move_camera() = 0;
  virtual void process_input(const InputEvent& event) = 0;
  virtual void init() = 0;
  virtual void end() = 0;

  std::unique_ptr<UserController> get_next_controller() {
    if (next_controller_ == nullptr)
      return nullptr;
    return std::move(next_controller_);
  }

protected:
  Sim& sim_;
  std::unique_ptr<UserController> next_controller_;
};

#endif