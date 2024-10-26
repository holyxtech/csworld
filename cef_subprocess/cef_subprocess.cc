#include <atomic>
#ifdef _WIN32
#include <windows.h>
#endif

#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "renderer_app.h"

#if defined(CEF_USE_SANDBOX)
#pragma comment(lib, "cef_sandbox.lib")
#endif

#ifdef _WIN32
int APIENTRY wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPTSTR lpCmdLine,
  int nCmdShow) {
#endif
#if defined(__linux__) || defined(__APPLE__)
int main(int argc, char** argv) {
#endif

  void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Provide CEF with command-line arguments.
#ifdef _WIN32
  CefMainArgs main_args(hInstance);
#endif
#if defined(__linux__) || defined(__APPLE__)
  CefMainArgs main_args(argc, argv);
#endif

  // Specify CEF global settings here.
  CefSettings settings;
  settings.command_line_args_disabled = true;

  CefRefPtr<RendererApp> app(new RendererApp());
  return CefExecuteProcess(main_args, app.get(), sandbox_info);
}
