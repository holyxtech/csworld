// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

namespace {

  SimpleHandler* g_instance = nullptr;

  // Returns a data: URI with the specified contents.
  std::string GetDataURI(const std::string& data, const std::string& mime_type) {
    return "data:" + mime_type + ";base64," +
           CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
  }

} // namespace

SimpleHandler::SimpleHandler() : width_(1024), height_(768) {
  DCHECK(!g_instance);
  g_instance = this;
}

SimpleHandler::~SimpleHandler() {
  g_instance = nullptr;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
  return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefWindow> window = browser_view->GetWindow();
    if (window) {
      window->SetTitle(title);
    }
  }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Sanity-check the configured runtime style.
  CHECK_EQ(CEF_RUNTIME_STYLE_ALLOY,
           browser->GetHost()->GetRuntimeStyle());

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }

  if (browser_list_.empty() && on_all_browsers_closed_) {
    on_all_browsers_closed_();
  }
}

void SimpleHandler::OnLoadError(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  ErrorCode errorCode,
  const CefString& errorText,
  const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED) {
    return;
  }

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::CloseAllBrowsers, this,
                                       force_close));
    return;
  }

  if (browser_list_.empty()) {
    return;
  }

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it) {
    (*it)->GetHost()->CloseBrowser(force_close);
  }
}

#if !defined(OS_MAC)
void SimpleHandler::PlatformShowWindow(CefRefPtr<CefBrowser> browser) {
  NOTIMPLEMENTED();
}
#endif

void SimpleHandler::SetOnAllBrowsersClosed(std::function<void()> callback) {
  on_all_browsers_closed_ = callback;
}

void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  rect.x = rect.y = 0;
  rect.width = width_;
  rect.height = height_;
}

void SimpleHandler::OnPaint(
  CefRefPtr<CefBrowser> browser,
  PaintElementType type,
  const RectList& dirtyRects,
  const void* buffer,
  int width,
  int height) {
  CEF_REQUIRE_UI_THREAD();

/*   if (type != PET_VIEW)
    return;
  if (width == width_ && height == height_) {
    for (auto& rect : dirtyRects) {
      for (int row = rect.y; row < std::min(height_, rect.y + rect.height); ++row) {
        std::memcpy(
          buffer_.data() + (row * width_ + rect.x) * 4,
          static_cast<const std::uint8_t*>(buffer) + (row * width + rect.x) * 4,
          rect.width * 4);
      }
    }
    new_frame_available_ = true;
  } */
}

bool SimpleHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  return false;
}

bool SimpleHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) {
  screenX = viewX;
  screenY = viewY;
  return true;
}

bool SimpleHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) {
  CefRect view_rect;
  GetViewRect(browser, view_rect);
  screen_info.device_scale_factor = 1.0f;
  screen_info.depth = 32;
  screen_info.depth_per_component = 8;
  screen_info.is_monochrome = false;
  screen_info.rect = view_rect;
  screen_info.available_rect = view_rect;
  return true;
}

void SimpleHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) {
  // Not implemented for basic usage
}

void SimpleHandler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) {
  // Not implemented for basic usage
}

void SimpleHandler::OnAcceleratedPaint(
  CefRefPtr<CefBrowser> browser,
  PaintElementType type,
  const RectList& dirtyRects,
  const CefAcceleratedPaintInfo &info) {
  // Not implemented for basic usage
}

bool SimpleHandler::StartDragging(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefDragData> drag_data,
  DragOperationsMask allowed_ops,
  int x, int y) {
  // Dragging is not supported
  return false;
}

void SimpleHandler::UpdateDragCursor(
  CefRefPtr<CefBrowser> browser,
  DragOperation operation) {
  // Not implemented for basic usage
}

void SimpleHandler::OnScrollOffsetChanged(
  CefRefPtr<CefBrowser> browser,
  double x,
  double y) {
  // Not implemented for basic usage
}

void SimpleHandler::OnImeCompositionRangeChanged(
  CefRefPtr<CefBrowser> browser,
  const CefRange& selected_range,
  const RectList& character_bounds) {
  // Not implemented for basic usage
}

void SimpleHandler::OnTextSelectionChanged(
  CefRefPtr<CefBrowser> browser,
  const CefString& selected_text,
  const CefRange& selected_range) {
  // Not implemented for basic usage
}

void SimpleHandler::OnVirtualKeyboardRequested(
  CefRefPtr<CefBrowser> browser,
  TextInputMode input_mode) {
  // Not implemented for basic usage
}
