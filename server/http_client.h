#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include <iostream>
#include <string>
#include <curl/curl.h>

class HTTPClient {

public:
  HTTPClient();
  ~HTTPClient();
  std::string make_request(const std::string& url);

private:
  static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);
  CURL* curl_;
};

#endif