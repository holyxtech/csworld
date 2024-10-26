// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "ui_client.h"
#include <sstream>
#include <string>
#include <GLFW/glfw3.h>
#include "options.h"
#include "shader_utils.h"

#include "dev_tools_handler.h"
#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "local_resource_handler.h"

namespace {

  UIClient* g_instance = nullptr;

  // Returns a data: URI with the specified contents.
  std::string GetDataURI(const std::string& data, const std::string& mime_type) {
    return "data:" + mime_type + ";base64," +
           CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
  }

} // namespace

// use width and height from options
UIClient::UIClient() : width_{Options::window_width}, height_{Options::window_height} {
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

UIClient::~UIClient() {
  glDeleteTextures(1, &texture_id_);
  g_instance = nullptr;
}

// static
UIClient* UIClient::GetInstance() {
  return g_instance;
}

void UIClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefWindow> window = browser_view->GetWindow();
    if (window) {
      window->SetTitle(title);
    }
  }
}

void UIClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
  if (!browser_) {
    browser_ = browser;
    // open devtools
    CefRefPtr<CefBrowserHost> host = browser_->GetHost();
    // Set up window information for DevTools popup
    CefWindowInfo windowInfo;
    windowInfo.SetAsPopup(nullptr, "DevTools");

    // Configure browser settings for DevTools
    CefBrowserSettings settings;
    // CefRefPtr<CefClient> devtools_handler = new DevToolsHandler(this);
    // why does the devtools first use this handler, and not the devtools_handler?

    host->ShowDevTools(windowInfo, nullptr, settings, CefPoint());
    /*     if (browser_created_promise_) {
          browser_created_promise_->set_value();
          browser_created_promise_ = nullptr;
        } */
  }
}

bool UIClient::DoClose(CefRefPtr<CefBrowser> browser) {
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

void UIClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty() && on_all_browsers_closed_) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
    on_all_browsers_closed_();
  }
}

void UIClient::OnLoadError(
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

void UIClient::OnLoadStart(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  TransitionType transition_type) {
  // close devtools if open
  // get url of frame
  CefString url = frame->GetURL();
  std::string url_str = url.ToString();
}

void UIClient::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&UIClient::CloseAllBrowsers, this,
                                       force_close));
    return;
  }

  if (browser_list_.empty()) {
    return;
  }

  // Create a copy of the browser list to iterate over
  BrowserList browsers_to_close = browser_list_;

  for (const auto& browser : browsers_to_close) {
    browser->GetHost()->CloseBrowser(force_close);
  }
}

void UIClient::OnBeforeDevToolsPopup(
  CefRefPtr<CefBrowser> browser,
  CefWindowInfo& windowInfo,
  CefRefPtr<CefClient>& client,
  CefBrowserSettings& settings,
  CefRefPtr<CefDictionaryValue>& extra_info,
  bool* use_default_window) {
  //*use_default_window = true;
}

void UIClient::SetOnAllBrowsersClosed(std::function<void()> callback) {
  on_all_browsers_closed_ = callback;
}

void UIClient::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  rect.x = rect.y = 0;
  rect.width = width_;
  rect.height = height_;
}

void UIClient::OnPaint(
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

void UIClient::render() {
  CEF_REQUIRE_UI_THREAD();
  glUseProgram(shader_);
  glBindTextureUnit(0, texture_id_);
  glUniform1i(glGetUniformLocation(shader_, "UIColor"), 0);

  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glBindTexture(GL_TEXTURE_2D, 0);
}

bool UIClient::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  return false;
}

bool UIClient::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) {
  screenX = viewX;
  screenY = viewY;
  return true;
}

bool UIClient::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) {
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

void UIClient::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) {
  // Not implemented for basic usage
}

void UIClient::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) {
  // Not implemented for basic usage
}

void UIClient::OnAcceleratedPaint(
  CefRefPtr<CefBrowser> browser,
  PaintElementType type,
  const RectList& dirtyRects,
  const CefAcceleratedPaintInfo& info) {
  // Not implemented for basic usage
}

bool UIClient::StartDragging(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefDragData> drag_data,
  DragOperationsMask allowed_ops,
  int x, int y) {
  return false;
}

void UIClient::UpdateDragCursor(
  CefRefPtr<CefBrowser> browser,
  DragOperation operation) {
  // Not implemented for basic usage
}

void UIClient::OnScrollOffsetChanged(
  CefRefPtr<CefBrowser> browser,
  double x,
  double y) {
  // Not implemented for basic usage
}

void UIClient::OnImeCompositionRangeChanged(
  CefRefPtr<CefBrowser> browser,
  const CefRange& selected_range,
  const RectList& character_bounds) {
  // Not implemented for basic usage
}

void UIClient::OnTextSelectionChanged(
  CefRefPtr<CefBrowser> browser,
  const CefString& selected_text,
  const CefRange& selected_range) {
  // Not implemented for basic usage
}

void UIClient::OnVirtualKeyboardRequested(
  CefRefPtr<CefBrowser> browser,
  TextInputMode input_mode) {
  // Not implemented for basic usage
}

void UIClient::OnMouseMove(int x, int y, bool mouseLeave) {
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

void UIClient::OnMouseButton(int x, int y, int button, bool down, int clickCount) {
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

void UIClient::OnMouseWheel(int x, int y, double deltaX, double deltaY) {
  if (!browser_)
    return;

  CefMouseEvent event;
  event.x = x;
  event.y = y;
  event.modifiers = 0;
  browser_->GetHost()->SendMouseWheelEvent(event, deltaX * 80, deltaY * 80);
}

void UIClient::OnKeyEvent(int key, bool down, int modifiers) {
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
  case GLFW_KEY_R:
    if (modifiers & GLFW_MOD_SHIFT) {
      browser_->Reload();
      return;
    }
  case GLFW_KEY_BACKSLASH: {
    if (browser_ && down) {
      CefRefPtr<CefBrowserHost> host = browser_->GetHost();
      // Set up window information for DevTools popup
      CefWindowInfo windowInfo;
      windowInfo.SetAsPopup(nullptr, "DevTools");

      // Configure browser settings for DevTools
      CefBrowserSettings settings;
      // CefRefPtr<CefClient> devtools_handler = new DevToolsHandler(this);
      // why does the devtools first use this handler, and not the devtools_handler?

      host->ShowDevTools(windowInfo, nullptr, settings, CefPoint());
    }
    return;
  } break;
  case GLFW_KEY_L: {
    if (down) {
      // Get the URL of the main frame
      CefString url = browser_->GetMainFrame()->GetURL();

      // Print or use the URL as needed
      std::string url_str = url.ToString();

      // get url of focused frame
      CefRefPtr<CefFrame> frame = browser_->GetFocusedFrame();
      CefString frame_url = frame->GetURL();
      std::string frame_url_str = frame_url.ToString();

      // get host
      CefRefPtr<CefBrowserHost> host = browser_->GetHost();

      // send message to the browser
      SendMessageToJS(nlohmann::json{{"type", "url"}, {"url", url_str}});
    }
    return;
  }
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

void UIClient::OnCharEvent(int char_code) {
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

bool UIClient::OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source) {
  if (source == FOCUS_SOURCE_SYSTEM || source == FOCUS_SOURCE_NAVIGATION) {
    // Allow the browser to take focus
    browser->GetHost()->SetFocus(true);
    return false; // Allow the focus to be set
  }
  return true; // Cancel setting focus for other sources
}

void UIClient::OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) {
  // The browser is losing focus. You might want to handle this if you have other UI elements.
}

CefRefPtr<CefResourceRequestHandler> UIClient::GetResourceRequestHandler(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  CefRefPtr<CefRequest> request,
  bool is_navigation,
  bool is_download,
  const CefString& request_initiator,
  bool& disable_default_handling) {

  // Check if the request is for our "fake" domain
  CefString url = request->GetURL();
  if (url.ToString().find("https://fake.domain.com") != std::string::npos) {
    disable_default_handling = true; // Prevent the default handling
    return this;                     // Return this class as the CefResourceRequestHandler
  }

  // Allow default handling for other URLs
  return nullptr;
}

CefRefPtr<CefResourceHandler> UIClient::GetResourceHandler(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  CefRefPtr<CefRequest> request) {

  std::string url = request->GetURL().ToString();

  // remove the prefix https://fake.domain.com/
  std::string suffix = url.substr(url.find("https://fake.domain.com/") + std::string("https://fake.domain.com/").length());
  std::string file_path = Options::instance()->get_ui_path(suffix);

  return new LocalResourceHandler(file_path);
}

bool UIClient::OnBeforeBrowse(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  CefRefPtr<CefRequest> request,
  bool user_gesture,
  bool is_redirect) {
  // get url of frame
  CefString url = frame->GetURL();
  std::string url_str = url.ToString();
  return false;
}

void UIClient::SendMessageToJS(const nlohmann::json& message) {

  // execute javascript in the browser
  CefRefPtr<CefFrame> frame = browser_->GetMainFrame();
  std::string message_str = "window.receiveMessage(" + message.dump() + ");";
  frame->ExecuteJavaScript(message_str, frame->GetURL(), 0);
}

bool UIClient::OnProcessMessageReceived(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  CefProcessId source_process,
  CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  // The first time the react app loads, it sends a message to the c++ side
  // to let us know it's ready to receive messages.
  if (browser_created_promise_) {
    browser_created_promise_->set_value();
    browser_created_promise_ = nullptr;
  }

  // Get the message name
  CefString name = message->GetName();
  std::string name_str = name.ToString();

  // Get the argument list as a dictionary
  if (!message->GetArgumentList()) {
    // Handle the case where there are no arguments
    return false;
  }

  message_queue_.enqueue(ConvertCefListToJson(message->GetArgumentList()));

  return true;
}

// Helper function to convert CefValue to JSON recursively
nlohmann::json UIClient::ConvertCefValueToJson(CefRefPtr<CefValue> value) {
  switch (value->GetType()) {
  case VTYPE_NULL:
    return nullptr;
  case VTYPE_BOOL:
    return value->GetBool();
  case VTYPE_INT:
    return value->GetInt();
  case VTYPE_DOUBLE:
    return value->GetDouble();
  case VTYPE_STRING:
    return value->GetString().ToString();
  case VTYPE_BINARY:
    // Handle binary data as needed
    return "binary_data";
  case VTYPE_DICTIONARY:
    return ConvertCefDictionaryToJson(value->GetDictionary());
  case VTYPE_LIST:
    return ConvertCefListToJson(value->GetList());
  default:
    return "unknown_type";
  }
}

// Helper function to convert CefDictionaryValue to JSON
nlohmann::json UIClient::ConvertCefDictionaryToJson(CefRefPtr<CefDictionaryValue> dict) {
  nlohmann::json json_obj;
  std::vector<CefString> keys;
  dict->GetKeys(keys);

  for (const auto& key : keys) {
    std::string key_str = key.ToString();
    if (dict->HasKey(key)) {
      json_obj[key_str] = ConvertCefValueToJson(dict->GetValue(key));
    }
  }

  return json_obj;
}

// Helper function to convert CefListValue to JSON
nlohmann::json UIClient::ConvertCefListToJson(CefRefPtr<CefListValue> list) {
  nlohmann::json json_array = nlohmann::json::array();
  for (size_t i = 0; i < list->GetSize(); ++i) {
    json_array.push_back(ConvertCefValueToJson(list->GetValue(i)));
  }
  return json_array;
}

void UIClient::SetBrowserCreatedPromise(std::promise<void>* promise) {
  browser_created_promise_ = promise;
}
