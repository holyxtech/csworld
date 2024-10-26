#ifndef CEFUI_H
#define CEFUI_H

#include <atomic>
#include <string>
#include "readerwriterqueue.h"
#include "nlohmann/json.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace cefui {

#ifdef _WIN32
  int Main(HINSTANCE hInstance);
#endif
#ifdef __linux__
  int Main(int argc, char** argv);
#endif

  void Shutdown();
  void DoMessageLoopWork();
  void Render();
  void OnMouseMove(int x, int y, bool mouseLeave);
  void OnMouseDown(int x, int y, int button);
  void OnMouseUp(int x, int y, int button);
  void OnMouseWheel(int x, int y, double deltaX, double deltaY);
  void OnKeyEvent(int key, bool down, int modifiers);
  void OnCharEvent(int char_code);
  void SendMessageToJS(const nlohmann::json& message);
  moodycamel::ReaderWriterQueue<nlohmann::json>& GetMessageQueue();

} // namespace cefui

#endif // CEFUI_H
