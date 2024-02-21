#include "http_client.h"

HTTPClient::HTTPClient() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl_ = curl_easy_init();

  if (!curl_) {
    throw std::runtime_error("Failed to initialize cURL");
  }

  curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
}

size_t HTTPClient::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
  size_t total_size = size * nmemb;
  output->append((char*)contents, total_size);
  return total_size;
}

HTTPClient::~HTTPClient() {
  curl_easy_cleanup(curl_);
  curl_global_cleanup();
}

std::string HTTPClient::make_request(const std::string& url) {
  std::string response_data;

  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data);

  CURLcode res = curl_easy_perform(curl_);

  if (res != CURLE_OK) {
    throw std::runtime_error(curl_easy_strerror(res));
  }

  return response_data;
}
