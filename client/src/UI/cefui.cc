
// cefui.cpp
#include <future>
#include <atomic>
#include "cefui.h"
#include "common.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "ui_app.h"
#include "ui_client.h"

namespace {
  std::atomic<bool> cef_shutdown_complete(false);
}

#if defined(CEF_USE_SANDBOX)
#pragma comment(lib, "cef_sandbox.lib")
#endif

namespace cefui {

#ifdef _WIN32
  int Main(HINSTANCE hInstance)
#elif defined(__APPLE__) || defined(__linux__)
  int Main(int argc, char** argv)
#endif
  {

#ifdef _WIN32
    CefMainArgs main_args(hInstance);
#elif defined(__APPLE__) || defined(__linux__)
    CefMainArgs main_args(argc, argv);
#endif
    void* sandbox_info = nullptr;
#if defined(CEF_USE_SANDBOX)
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif

    CefSettings settings;
    settings.command_line_args_disabled = true;

    std::string dir = common::get_working_dir();
    std::string exe_path = dir + "/" + CEF_SUBPROCESS_NAME_WITH_EXT;
    std::wstring wdir(dir.begin(), dir.end());
    std::wstring wexe_path(exe_path.begin(), exe_path.end());
    CefString cef_subprocess_path(wexe_path);
    cef_string_copy(cef_subprocess_path.GetStruct()->str, cef_subprocess_path.GetStruct()->length, &settings.browser_subprocess_path);
    std::wstring log_path = wdir + L"\\cef_log.log";
    CefString cef_log_path(log_path);
    cef_string_copy(cef_log_path.GetStruct()->str, cef_log_path.GetStruct()->length, &settings.log_file);

    settings.chrome_runtime = true;
    settings.windowless_rendering_enabled = true;
    settings.multi_threaded_message_loop = false;

#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    // Set up the promise and future
    std::promise<void> browser_created_promise;
    std::future<void> browser_created_future = browser_created_promise.get_future();

    CefRefPtr<UIApp> app = new UIApp(cef_shutdown_complete, browser_created_promise);
    if (!CefInitialize(main_args, settings, app.get(), sandbox_info)) {
      return CefGetExitCode();
    }

    // Do message loop work and wait for the browser to be created
    while (browser_created_future.wait_for(std::chrono::milliseconds(32)) == std::future_status::timeout) {
      CefDoMessageLoopWork();
    }

    
    return 0;
  }

  void Shutdown() {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler) {
      handler->CloseAllBrowsers(true);
      while (!cef_shutdown_complete) {
        CefDoMessageLoopWork();
      }
    }
    CefShutdown();
  }

  void DoMessageLoopWork() {
    if (!cef_shutdown_complete)
      CefDoMessageLoopWork();
  }

  void Render() {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      handler->render();
  }

  void OnMouseMove(int x, int y, bool mouseLeave) {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      handler->OnMouseMove(x, y, mouseLeave);
  }

  void OnMouseDown(int x, int y, int button) {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      handler->OnMouseButton(x, y, button, true, 1);
  }

  void OnMouseUp(int x, int y, int button) {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      handler->OnMouseButton(x, y, button, false, 1);
  }

  void OnMouseWheel(int x, int y, double deltaX, double deltaY) {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      handler->OnMouseWheel(x, y, deltaX, deltaY);
  }

  void OnKeyEvent(int key, bool down, int modifiers) {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      handler->OnKeyEvent(key, down, modifiers);
  }

  void OnCharEvent(int char_code) {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      handler->OnCharEvent(char_code);
  }

  void SendMessageToJS(const nlohmann::json& message) {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      handler->SendMessageToJS(message);
  }

  moodycamel::ReaderWriterQueue<nlohmann::json>& GetMessageQueue() {
    CefRefPtr<UIClient> handler = UIClient::GetInstance();
    if (handler)
      return handler->GetMessageQueue();
    else
      throw std::runtime_error("UIClient not initialized");
  }

} // namespace cefui
