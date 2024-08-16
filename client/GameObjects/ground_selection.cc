#include "ground_selection.h"
#include "../input.h"

void GroundSelection::process_input(const InputEvent& event) {
  switch (event.kind) {
  case InputEvent::MouseButtonEvent: {
    auto& mouse_button_event = std::any_cast<const MouseButtonEvent&>(event.data);
    if (mouse_button_event.action == GLFW_PRESS && mouse_button_event.button != GLFW_MOUSE_BUTTON_LEFT)
      doing_selection_ = true;
    else if (mouse_button_event.action == GLFW_RELEASE && mouse_button_event.button == GLFW_MOUSE_BUTTON_LEFT)
      doing_selection_ = false;
  } break;
  }
}
void GroundSelection::step() {
  if (!doing_selection_)
    return;
  auto [xpos, ypos] = Input::instance()->get_cursor_pos();
  // 1. update int3d list
  // 2. mesh/update scene component
}