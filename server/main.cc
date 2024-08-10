#ifdef _WIN32
#include <SDKDDKVer.h>
#endif
#include <array>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <asio.hpp>
#include <boost/bind/bind.hpp>
#include "config.h"
#include "readerwriterqueue.h"
#include "chunk.h"
#include "sim_server.h"
#include "tcp_server.h"
#include "world_generator.h"

int main() {
  std::filesystem::create_directory(std::string(APPLICATION_DATA_DIR) + "/images/");
  std::filesystem::create_directory(std::string(APPLICATION_DATA_DIR) + "/images/landcover/");
  std::filesystem::create_directory(std::string(APPLICATION_DATA_DIR) + "/images/elevation/");

  asio::io_context io_context;
  TCPServer tcp_server(io_context);
  asio::thread t(boost::bind(&asio::io_context::run, &io_context));
  SimServer sim_server(tcp_server);
  while (true) {
    sim_server.step();
  }

  return 0;
}