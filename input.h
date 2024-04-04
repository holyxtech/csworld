#ifndef INPUT_H
#define INPUT_H

#include <array>
#include <GLFW/glfw3.h>
#include "readerwriterqueue.h"

struct MouseButtonEvent {
  int button;
  int action;
  int mods;
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
  void set_cursor_start_pos(double xpos, double ypos);
  const std::array<double, 2>& get_cursor_pos() const;
  int get_last_mouse_button_state(int button) const;
  moodycamel::ReaderWriterQueue<MouseButtonEvent>& get_mouse_button_events();

private:
  std::array<double, 2> cursor_pos_;
  std::array<int, 8> mouse_button_state_;

  moodycamel::ReaderWriterQueue<MouseButtonEvent> mouse_button_events_;
};

#endif