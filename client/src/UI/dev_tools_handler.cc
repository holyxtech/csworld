// dev_tools_handler.cpp
#include "dev_tools_handler.h"
#include "ui_client.h"

DevToolsHandler::DevToolsHandler(UIClient* handler)
    : client_(handler) {}

/* CefRefPtr<CefLifeSpanHandler> DevToolsHandler::GetLifeSpanHandler() {
  return client_;
} */
/* 
CefRefPtr<CefRequestHandler> DevToolsHandler::GetRequestHandler() {
  return client_;
}
 */