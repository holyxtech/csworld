// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_ui_app_H_
#define CEF_TESTS_CEFSIMPLE_ui_app_H_

#include "include/cef_app.h"
#include <atomic>
#include <future>

// Implement application-level callbacks for the browser process.
class UIApp
    : public CefApp,
      public CefBrowserProcessHandler {
public:
  UIApp(std::atomic<bool>& cef_shutdown_complete, std::promise<void>& browser_created_promise);

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;
  CefRefPtr<CefClient> GetDefaultClient() override;

private:
  std::atomic<bool>& cef_shutdown_complete_;
  std::promise<void>& browser_created_promise_;
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(UIApp);
};

#endif // CEF_TESTS_CEFSIMPLE_ui_app_H_
