// copyright (c) 2013 the chromium embedded framework authors. all rights
// reserved. use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_app.h"

#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "simple_handler.h"

SimpleApp::SimpleApp(std::atomic<bool>& cef_shutdown_complete)
    : cef_shutdown_complete_(cef_shutdown_complete) {}

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefBrowserSettings browser_settings;
  browser_settings.windowless_frame_rate = 60;
  CefWindowInfo window_info;
  window_info.SetAsWindowless(nullptr);
  cef_runtime_style_t runtime_style = CEF_RUNTIME_STYLE_ALLOY;
  window_info.runtime_style = runtime_style;

  // SimpleHandler implements browser-level callbacks.
  CefRefPtr<SimpleHandler> handler(new SimpleHandler());

  auto cef_shutdown_lambda = [this]() { cef_shutdown_complete_ = true; };
  handler->SetOnAllBrowsersClosed(cef_shutdown_lambda);

  std::string url = "https://www.google.com";

  CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, nullptr, nullptr);
}

CefRefPtr<CefClient> SimpleApp::GetDefaultClient() {
  // Called when a new browser window is created via the Chrome runtime UI.
  return SimpleHandler::GetInstance();
}
