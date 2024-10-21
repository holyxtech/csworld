// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"
#include <sstream>
#include <string>
#include <GLFW/glfw3.h>
#include "options.h"
#include "shader_utils.h"

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

SimpleHandler::SimpleHandler() : width_{1280}, height_{720} {
  DCHECK(!g_instance);
  g_instance = this;
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  shader_ = shader_utils::create_shader("quad.vs", "ui.fs");
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  glBindVertexArray(0);
}

SimpleHandler::~SimpleHandler() {
  glDeleteTextures(1, &texture_id_);
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
  browser_ = browser;
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
  if (type != PET_VIEW)
    return;

  int old_width = width_;
  int old_height = height_;
  width_ = width;
  height_ = height;

  glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

  if (width != old_width || height != old_height ||
      (dirtyRects.size() == 1 &&
       dirtyRects[0] == CefRect(0, 0, width, height))) {
    // upload height uniform to shader
    glUseProgram(shader_);
    glUniform1i(glGetUniformLocation(shader_, "height"), Options::window_height);
    glUseProgram(0);

    // Resize the texture
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
  } else {
    // Partial updates

    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    for (const auto& rect : dirtyRects) {
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y);
      glTexSubImage2D(
        GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height,
        GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
    }
  }
}

void SimpleHandler::render() {
  CEF_REQUIRE_UI_THREAD();
  glUseProgram(shader_);
  glBindTextureUnit(0, texture_id_);
  glUniform1i(glGetUniformLocation(shader_, "UIColor"), 0);

  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glBindTexture(GL_TEXTURE_2D, 0);
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
  const CefAcceleratedPaintInfo& info) {
  // Not implemented for basic usage
}

bool SimpleHandler::StartDragging(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefDragData> drag_data,
  DragOperationsMask allowed_ops,
  int x, int y) {
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

void SimpleHandler::OnMouseMove(int x, int y, bool mouseLeave) {
  if (!browser_)
    return;

  CefMouseEvent event;
  event.x = x;
  event.y = y;
  event.modifiers = 0;

  if (is_mouse_down_) {
    event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
  }

  browser_->GetHost()->SendMouseMoveEvent(event, mouseLeave);
}

void SimpleHandler::OnMouseButton(int x, int y, int button, bool down, int clickCount) {
  if (!browser_)
    return;

  CefMouseEvent event;
  event.x = x;
  event.y = y;
  event.modifiers = 0;

  CefBrowserHost::MouseButtonType btnType;
  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    btnType = MBT_LEFT;
    break;
  case GLFW_MOUSE_BUTTON_MIDDLE:
    btnType = MBT_MIDDLE;
    break;
  case GLFW_MOUSE_BUTTON_RIGHT:
    btnType = MBT_RIGHT;
    break;
  default:
    return;
  }

  is_mouse_down_ = down; // Update the mouse button state

  if (down) {
    browser_->GetHost()->SendMouseClickEvent(event, btnType, false, clickCount);
    // browser_->GetHost()->SetFocus(true);
  } else {
    browser_->GetHost()->SendMouseClickEvent(event, btnType, true, clickCount);
  }
}

void SimpleHandler::OnMouseWheel(int x, int y, double deltaX, double deltaY) {
  if (!browser_)
    return;

  CefMouseEvent event;
  event.x = x;
  event.y = y;
  event.modifiers = 0;
  browser_->GetHost()->SendMouseWheelEvent(event, deltaX*80, deltaY*80);
}

void SimpleHandler::OnKeyEvent(int key, bool down, int modifiers) {
  if (!browser_)
    return;    
  int code = 0;
  switch (key) {
  case GLFW_KEY_BACKSPACE:
    code = 0x08; // VK_BACK
    break;
  case GLFW_KEY_TAB:
    code = 0x09; // VK_TAB
    break;
  case GLFW_KEY_ENTER:
    code = 0x0D; // VK_RETURN
    break;
  case GLFW_KEY_ESCAPE:
    code = 0x1B; // VK_ESCAPE
    break;
  default:
    return; // Do nothing for other keys
  }    

  CefKeyEvent event;
  event.windows_key_code = code;
  event.native_key_code = code;
  event.is_system_key = false;
  event.modifiers = modifiers;

  if (down) {
    event.type = KEYEVENT_RAWKEYDOWN;
  } else {
    event.type = KEYEVENT_KEYUP;
  }

  browser_->GetHost()->SendKeyEvent(event);
}

void SimpleHandler::OnCharEvent(int char_code) {
  if (!browser_)
    return;

  CefKeyEvent event;
  event.type = KEYEVENT_CHAR;
  event.is_system_key = false;
  event.modifiers = 0;
  event.windows_key_code = char_code;
  event.native_key_code = char_code;

  browser_->GetHost()->SendKeyEvent(event);
}

bool SimpleHandler::OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source) {
  if (source == FOCUS_SOURCE_SYSTEM || source == FOCUS_SOURCE_NAVIGATION) {
    // Allow the browser to take focus
    browser->GetHost()->SetFocus(true);
    return false; // Allow the focus to be set
  }
  return true; // Cancel setting focus for other sources
}

void SimpleHandler::OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) {
  // The browser is losing focus. You might want to handle this if you have other UI elements.
}
