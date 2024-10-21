// Include necessary Windows and CEF headers
#include <windows.h>
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "simple_app.h"
#include "simple_handler.h"
#include "common.h"
namespace {
  // Global pointer to the SimpleApp instance
  CefRefPtr<SimpleApp> app;
  // Atomic flag to indicate when CEF shutdown is complete
  std::atomic<bool> cef_shutdown_complete(false);
} // namespace

#if defined(CEF_USE_SANDBOX)
// Link the CEF sandbox library if sandbox is enabled
#pragma comment(lib, "cef_sandbox.lib")
#endif

namespace cefui {
  // Main entry point for CEF initialization
  int main(HINSTANCE hInstance) {
    void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
    // Initialize sandbox information for Windows
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif
 
    // Create the main arguments for CEF
    CefMainArgs main_args(hInstance);

    // Configure CEF settings
    CefSettings settings;
    settings.command_line_args_disabled = true;

    // Get the path to the CEF subprocess executable
    std::string dir = common::get_working_dir();
    std::string exe_path = dir + "/" + CEF_SUBPROCESS_NAME_WITH_EXT;
    std::wstring wdir(dir.begin(), dir.end());
    std::wstring wexe_path(exe_path.begin(), exe_path.end());
    CefString cef_subprocess_path(wexe_path);
    cef_string_copy(cef_subprocess_path.GetStruct()->str, cef_subprocess_path.GetStruct()->length, &settings.browser_subprocess_path);
    std::wstring log_path = wdir + L"\\cef_log.log";
    CefString cef_log_path(log_path);
    cef_string_copy(cef_log_path.GetStruct()->str, cef_log_path.GetStruct()->length, &settings.log_file);

    // Enable Chrome runtime and windowless rendering
    settings.chrome_runtime = true;
    settings.windowless_rendering_enabled = true;
    settings.multi_threaded_message_loop = false;
    // settings.background_color = 0x00000000; // Transparent background (ARGB format)

#if !defined(CEF_USE_SANDBOX)
    // Disable the sandbox if not defined
    settings.no_sandbox = true;
#endif

    // Create and initialize the SimpleApp instance
    app = new SimpleApp(cef_shutdown_complete);
    if (!CefInitialize(main_args, settings, app.get(), sandbox_info)) {
      return CefGetExitCode();
    }

    return 0;
  }

  // Shutdown CEF and clean up resources
  void shutdown() {

    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    if (handler) {
      handler->CloseAllBrowsers(true);
      // Wait for all browsers to close
      while (!cef_shutdown_complete) {
        CefDoMessageLoopWork();
      }
    }

    CefShutdown();
  }

  // Process CEF messages
  void domessage() {
    if (!cef_shutdown_complete)
      CefDoMessageLoopWork();
  }

  // render from simplerhandler
  void render() {
    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    if (handler)
      handler->render();
  }

  // onmousemove
  void onmousemove(int x, int y, bool mouseLeave) {
    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    if (handler)
      handler->OnMouseMove(x, y, mouseLeave);
  }
  // on mouse button
  void onmousedown(int x, int y, int button) {
    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    if (handler)
      handler->OnMouseButton(x, y, button, true, 1);
  }

  // on mouse up
  void onmouseup(int x, int y, int button) {
    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    if (handler)
      handler->OnMouseButton(x, y, button, false, 1);
  }
  // on mouse wheel
  void onmousewheel(int x, int y, double deltaX, double deltaY) {
    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    if (handler)
      handler->OnMouseWheel(x, y, deltaX, deltaY);
  }
  // on key event
  void onkeyevent(int key, bool down, int modifiers) {
    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    if (handler)
      handler->OnKeyEvent(key, down, modifiers);
  }
  // on char event
  void oncharevent(int char_code) {
    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    if (handler)
      handler->OnCharEvent(char_code);
  }
} // namespace cefui
