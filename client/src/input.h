#ifndef INPUT_H
#define INPUT_H

#include <any>
#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "readerwriterqueue.h"

struct MouseButtonEvent {
  int button;
  int action;
  int mods;
  double xpos;
  double ypos;
};

struct KeyButtonEvent {
  int key;
  int action;
};

struct InputEvent {
  enum Kind {
    MouseButtonEvent,
    KeyButtonEvent,
  };
  Kind kind;
  std::any data;
  template <typename T>
  InputEvent(Kind k, const T& obj) : kind(k), data(obj) {}
};

class Input {
public:
  static Input* instance() {
    static Input* instance = new Input();
    return instance;
  }

  void key_callback(GLFWwindow* window, int key, int action);
  void mouse_button_callback(int button, int action, int mods);
  void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
  const std::array<double, 2>& get_cursor_pos() const;
  const std::array<double, 2>& get_prev_cursor_pos() const;
  void set_cursor_pos(double xpos, double ypos);
  void set_prev_cursor_pos(double xpos, double ypos);
  int get_last_mouse_button_state(int button) const;
  moodycamel::ReaderWriterQueue<MouseButtonEvent>& get_mouse_button_events();
  moodycamel::ReaderWriterQueue<KeyButtonEvent>& get_key_button_events();
  bool is_mouse_button_pressed(int button) const;

private:
  std::array<double, 2> prev_cursor_pos_;
  std::array<double, 2> cursor_pos_;
  std::array<int, 8> mouse_button_state_;
  std::array<bool, GLFW_MOUSE_BUTTON_LAST> pressed_mouse_buttons_;

  moodycamel::ReaderWriterQueue<MouseButtonEvent> mouse_button_events_;
  moodycamel::ReaderWriterQueue<KeyButtonEvent> key_button_events_;
};

#endif