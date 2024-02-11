#ifndef INPUT_H
#define INPUT_H

#include <array>
#include <GLFW/glfw3.h>

class Input {
public:
  static Input* instance() {
    static Input* instance = new Input();
    return instance;
  }

  void key_callback(GLFWwindow* window, int key, int action);
  void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
  void set_cursor_start_pos(double xpos, double ypos);
  const std::array<double, 2>& get_cursor_start_pos() const;

private:
  std::array<double, 2> cursor_start_pos_;
};

#endif