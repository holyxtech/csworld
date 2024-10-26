#include <algorithm>
#include <fstream>
#include <sstream>
#include "local_resource_handler.h"

LocalResourceHandler::LocalResourceHandler(const std::string& file_path)
    : file_path_(file_path), offset_(0) {}

bool LocalResourceHandler::Open(
  CefRefPtr<CefRequest> request,
  bool& handle_request,
  CefRefPtr<CefCallback> callback) {
  // Read the file immediately and handle the request
  std::ifstream file(file_path_, std::ios::binary);
  if (file) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    file_data_ = buffer.str();
    handle_request = true;
    return true;
  } else {
    handle_request = true;
    return false; // Cancel the request if the file couldn't be opened
  }
}

void LocalResourceHandler::GetResponseHeaders(
  CefRefPtr<CefResponse> response,
  int64_t& response_length,
  CefString& redirectUrl) {
  response->SetStatus(200);

  if (file_path_.find(".html") != std::string::npos) {
    response->SetMimeType("text/html");
  } else if (file_path_.find(".js") != std::string::npos) {
    response->SetMimeType("application/javascript");
  } else if (file_path_.find(".css") != std::string::npos) {
    response->SetMimeType("text/css");
  } else if (file_path_.find(".png") != std::string::npos) {
    response->SetMimeType("image/png");
  } else if (file_path_.find(".jpg") != std::string::npos || file_path_.find(".jpeg") != std::string::npos) {
    response->SetMimeType("image/jpeg");
  } else if (file_path_.find(".gif") != std::string::npos) {
    response->SetMimeType("image/gif");
  } else if (file_path_.find(".svg") != std::string::npos) {
    response->SetMimeType("image/svg+xml");
  } else {
    response->SetMimeType("text/plain");
  }

  response_length = file_data_.size();
}

bool LocalResourceHandler::ReadResponse(
  void* data_out, int bytes_to_read, int& bytes_read,
  CefRefPtr<CefCallback> callback) {
  int remaining = file_data_.size() - offset_;
  int bytes_to_copy = std::min(bytes_to_read, remaining);

  if (bytes_to_copy > 0) {
    std::memcpy(data_out, file_data_.data() + offset_, bytes_to_copy);
    offset_ += bytes_to_copy;
    bytes_read = bytes_to_copy;
    return true;
  }

  bytes_read = 0;
  return false;
}

void LocalResourceHandler::Cancel() {}
