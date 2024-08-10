#include "input.h"
#include <iostream>

void Input::mouse_button_callback(int button, int action, int mods) {
  mouse_button_events_.enqueue(MouseButtonEvent{button, action, mods});
}

void Input::key_callback(GLFWwindow* window, int key, int action) {
  key_button_events_.enqueue(KeyButtonEvent{key, action});

  if (action == GLFW_PRESS)
    pressed_keys_[key] = true;
  else if (action == GLFW_RELEASE)
    pressed_keys_[key] = false;
}

void Input::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  cursor_pos_[0] = xpos;
  cursor_pos_[1] = ypos;
}

const std::array<double, 2>& Input::get_cursor_pos() const {
  return cursor_pos_;
}

const std::array<double, 2>& Input::get_prev_cursor_pos() const {
  return prev_cursor_pos_;
}

void Input::set_cursor_pos(double xpos, double ypos) {
  cursor_pos_[0] = xpos;
  cursor_pos_[1] = ypos;
}

void Input::set_prev_cursor_pos(double xpos, double ypos) {
  prev_cursor_pos_[0] = xpos;
  prev_cursor_pos_[1] = ypos;
}

int Input::get_last_mouse_button_state(int button) const {
  return mouse_button_state_[button];
}

moodycamel::ReaderWriterQueue<MouseButtonEvent>& Input::get_mouse_button_events() {
  return mouse_button_events_;
}

moodycamel::ReaderWriterQueue<KeyButtonEvent>& Input::get_key_button_events() {
  return key_button_events_;
}

bool Input::is_key_pressed(int key) const {
  return pressed_keys_[key];
}