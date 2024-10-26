#include "renderer_app.h"
#include "v8_handler.h"

void RendererApp::OnContextCreated(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  CefRefPtr<CefV8Context> context) {

  browser_list_.push_back(browser);
  if (!browser_) {
    browser_ = browser;
  }

  CefRefPtr<CefV8Value> global = context->GetGlobal();
  CefRefPtr<CefV8Handler> handler = new V8Handler(browser); // Pass the browser reference
  CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction("sendMessage", handler);
  global->SetValue("sendMessage", func, V8_PROPERTY_ATTRIBUTE_NONE);
}

bool RendererApp::OnProcessMessageReceived(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  CefProcessId source_process,
  CefRefPtr<CefProcessMessage> message) {
  return false;
}

void RendererApp::OnWebKitInitialized() {
  // Called after WebKit has been initialized.
}
