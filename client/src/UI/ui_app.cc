#include "ui_app.h"

#include <string>

#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"
#include "options.h"
#include "ui_client.h"

UIApp::UIApp(std::atomic<bool>& cef_shutdown_complete, std::promise<void>& browser_created_promise)
    : cef_shutdown_complete_(cef_shutdown_complete), browser_created_promise_(browser_created_promise) {
}

void UIApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefBrowserSettings browser_settings;
  browser_settings.windowless_frame_rate = 60;
  CefWindowInfo window_info;
  window_info.SetAsWindowless(nullptr);
  cef_runtime_style_t runtime_style = CEF_RUNTIME_STYLE_ALLOY;
  window_info.runtime_style = runtime_style;

  // UIClient implements browser-level callbacks.
  CefRefPtr<UIClient> handler = new UIClient();
  auto cef_shutdown_lambda = [this]() { cef_shutdown_complete_ = true; };
  handler->SetOnAllBrowsersClosed(cef_shutdown_lambda);
  handler->SetBrowserCreatedPromise(&browser_created_promise_);
  // Get the path to the local index.html file
  // use fake domain to avoid CORS
  CefString url("https://fake.domain.com/" + Options::instance()->get_ui_path("index.html"));
  //CefString url("https://youtube.com");

  // Create the browser with the request
  CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, nullptr, nullptr);
}

CefRefPtr<CefClient> UIApp::GetDefaultClient() {
  // Called when a new browser window is created via the Chrome runtime UI.
  return UIClient::GetInstance();
}