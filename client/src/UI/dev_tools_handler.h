// dev_tools_handler.h
#ifndef DEV_TOOLS_HANDLER_H
#define DEV_TOOLS_HANDLER_H

#include "include/cef_client.h"

class UIClient;

class DevToolsHandler : public CefClient {
public:
  DevToolsHandler(UIClient* handler);

  // Implement necessary CefClient methods for DevTools
  //CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
  //CefRefPtr<CefRequestHandler> GetRequestHandler() override;

private:
  UIClient* client_;

  IMPLEMENT_REFCOUNTING(DevToolsHandler);
};

#endif // DEV_TOOLS_HANDLER_H

