// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#include <functional>
#include <list>
#include <GL/glew.h>

#include "include/cef_client.h"
#include "include/cef_render_handler.h"

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRenderHandler,
                      public CefFocusHandler {
public:
  explicit SimpleHandler();
  ~SimpleHandler() override;

  // Provide access to the single global instance of this object.
  static SimpleHandler* GetInstance();

  // CefClient methods:
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
  CefRefPtr<CefFocusHandler> GetFocusHandler() override { return this; }
  // CefDisplayHandler methods:
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString& title) override;

  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods:
  void OnLoadError(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   ErrorCode errorCode,
                   const CefString& errorText,
                   const CefString& failedUrl) override;

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);
  bool IsClosing() const { return is_closing_; }
  void SetOnAllBrowsersClosed(std::function<void()> callback);

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

  // Add these method declarations in the SimpleHandler class
  void OnMouseMove(int x, int y, bool mouseLeave);
  void OnMouseButton(int x, int y, int button, bool down, int clickCount);
  void OnMouseWheel(int x, int y, double deltaX, double deltaY);
  void OnKeyEvent(int key, bool down, int modifiers);
  void OnCharEvent(int char_code);

  bool OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source) override;
  void OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) override;

private:
  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);
  void PlatformShowWindow(CefRefPtr<CefBrowser> browser);

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;
  std::function<void()> on_all_browsers_closed_;

  bool new_frame_available_ = false;
  bool is_mouse_down_ = false;
  bool is_closing_ = false;
  int width_, height_;
  std::vector<uint8_t> buffer_;
  GLuint texture_id_;
  GLuint shader_;
  GLuint vao_;
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);

  // Add this member variable to the SimpleHandler class
  CefRefPtr<CefBrowser> browser_;
};

#endif // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
