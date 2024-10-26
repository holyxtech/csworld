#ifndef LOCAL_RESOURCE_HANDLER_H
#define LOCAL_RESOURCE_HANDLER_H

#include <string>
#include "include/cef_base.h"
#include "include/cef_resource_handler.h"

class LocalResourceHandler : public CefResourceHandler {
public:
  LocalResourceHandler(const std::string& file_path);

  bool Open(
    CefRefPtr<CefRequest> request,
    bool& handle_request,
    CefRefPtr<CefCallback> callback) override;
  void GetResponseHeaders(
    CefRefPtr<CefResponse> response,
    int64_t& response_length,
    CefString& redirectUrl) override;
  bool ReadResponse(
    void* data_out,
    int bytes_to_read,
    int& bytes_read,
    CefRefPtr<CefCallback> callback) override;
  void Cancel() override;

private:
  std::string file_path_;
  std::string file_data_;
  std::size_t offset_;

  IMPLEMENT_REFCOUNTING(LocalResourceHandler);
};

#endif // LOCAL_RESOURCE_HANDLER_H
