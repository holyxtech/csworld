#include "input.h"
#include <iostream>

void Input::key_callback(GLFWwindow* window, int key, int action) {
  //std::cout << key << "," << action << std::endl;
}

void Input::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
}

void Input::set_cursor_start_pos(double xpos, double ypos) {
  cursor_start_pos_[0] = xpos;
  cursor_start_pos_[1] = ypos;
}

const std::array<double, 2>& Input::get_cursor_start_pos() const {
  return cursor_start_pos_;
}