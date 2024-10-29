// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_ui_client_H_
#define CEF_TESTS_CEFSIMPLE_ui_client_H_


#ifdef _WIN32
#include <windows.h>
#endif

#include <functional>
#include <list>
#include <GL/glew.h>
#include <string>
#include <atomic>
#include <future>

#include "dev_tools_handler.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#include "readerwriterqueue.h"
#include "nlohmann/json.hpp"
#include <glm/glm.hpp>
class UIClient
    : public CefClient,
      public CefDisplayHandler,
      public CefLifeSpanHandler,
      public CefLoadHandler,
      public CefRenderHandler,
      public CefFocusHandler,
      public CefRequestHandler,
      public CefResourceRequestHandler {
public:
  explicit UIClient();
  ~UIClient() override;

  // Provide access to the single global instance of this object.
  static UIClient* GetInstance();

  // CefClient methods:
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
  CefRefPtr<CefFocusHandler> GetFocusHandler() override { return this; }
  CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
  bool OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) override;

  // CefDisplayHandler methods:
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString& title) override;

  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeDevToolsPopup(
    CefRefPtr<CefBrowser> browser,
    CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings,
    CefRefPtr<CefDictionaryValue>& extra_info,
    bool* use_default_window) override;

  // CefLoadHandler methods:
  void OnLoadError(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    ErrorCode errorCode,
    const CefString& errorText,
    const CefString& failedUrl) override;

  void OnLoadStart(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    TransitionType transition_type) override;

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);
  bool IsClosing() const { return is_closing_; }
  void SetOnAllBrowsersClosed(std::function<void()> callback);
  void SetBrowserCreatedPromise(std::promise<void>* promise);

  // CefRenderHandler methods:
  bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
  void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
  bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) override;
  bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;
  void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
  void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
  void OnPaint(
    CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    const void* buffer,
    int width,
    int height) override;
  void OnAcceleratedPaint(
    CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    const CefAcceleratedPaintInfo& info) override;
  bool StartDragging(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDragData> drag_data,
    DragOperationsMask allowed_ops,
    int x, int y) override;
  void UpdateDragCursor(
    CefRefPtr<CefBrowser> browser,
    DragOperation operation) override;
  void OnScrollOffsetChanged(
    CefRefPtr<CefBrowser> browser,
    double x,
    double y) override;
  void OnImeCompositionRangeChanged(
    CefRefPtr<CefBrowser> browser,
    const CefRange& selected_range,
    const RectList& character_bounds) override;
  void OnTextSelectionChanged(
    CefRefPtr<CefBrowser> browser,
    const CefString& selected_text,
    const CefRange& selected_range) override;
  void OnVirtualKeyboardRequested(
    CefRefPtr<CefBrowser> browser,
    TextInputMode input_mode) override;

  void render();

  // Add these method declarations in the UIClient class
  void OnMouseMove(int x, int y, bool mouseLeave);
  void OnMouseButton(int x, int y, int button, bool down, int clickCount);
  void OnMouseWheel(int x, int y, double deltaX, double deltaY);
  void OnKeyEvent(int key, bool down, int modifiers);
  void OnCharEvent(int char_code);

  bool OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source) override;
  void OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) override;

  CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool is_navigation,
    bool is_download,
    const CefString& request_initiator,
    bool& disable_default_handling) override;
  CefRefPtr<CefResourceHandler> GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request) override;
  bool OnBeforeBrowse(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool user_gesture,
    bool is_redirect) override;

  void SendMessageToJS(const nlohmann::json& message);

  moodycamel::ReaderWriterQueue<nlohmann::json>& GetMessageQueue() {
    return message_queue_;
  }

private:
  // Platform-specific implementation.

#ifdef _WIN32
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) {
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
    if (hwnd) {
      SetWindowText(hwnd, std::wstring(title).c_str());
    }
  }
#endif

#ifndef __APPLE__
  void PlatformShowWindow(CefRefPtr<CefBrowser> browser) {
    NOTIMPLEMENTED();
  }
#endif

  nlohmann::json ConvertCefDictionaryToJson(CefRefPtr<CefDictionaryValue> dict);
  nlohmann::json ConvertCefListToJson(CefRefPtr<CefListValue> list);
  nlohmann::json ConvertCefValueToJson(CefRefPtr<CefValue> value);

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;
  CefRefPtr<CefBrowser> browser_;
  std::function<void()> on_all_browsers_closed_;
  std::promise<void>* browser_created_promise_ = nullptr;

  bool new_frame_available_ = false;
  bool is_mouse_down_ = false;
  bool is_closing_ = false;
  int width_, height_;
  std::vector<uint8_t> buffer_;
  GLuint texture_id_;
  GLuint shader_;
  GLuint vao_;
  
  GLuint drag_texture_id_ = 0;
  GLuint drag_shader_ = 0;
  GLuint drag_vao_ = 0;
  glm::vec2 drag_position_; // To store the current drag position
  glm::vec2 drag_size_; // To store the current drag size
  glm::vec2 image_size_; // To store the current image size

  bool dragging_ = false;

  moodycamel::ReaderWriterQueue<nlohmann::json> message_queue_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(UIClient);
};

#endif // CEF_TESTS_CEFSIMPLE_ui_client_H_
