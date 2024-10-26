#ifndef RENDERER_APP_H
#define RENDERER_APP_H

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"

class RendererApp : public CefApp, public CefRenderProcessHandler {
public:
  RendererApp() = default;

  // CefApp methods:
  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
    return this;
  }

  // CefRenderProcessHandler methods:
  void OnContextCreated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) override;
  bool OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) override;
  void OnWebKitInitialized() override;

private:
  // browser list
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;
  CefRefPtr<CefBrowser> browser_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(RendererApp);
};

#endif // RENDERER_APP_H
