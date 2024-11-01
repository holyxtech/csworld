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
#include "readerwriterqueue.h"
#include "chunk.h"
#include "sim_server.h"
#include "tcp_server.h"
#include "world_generator.h"

int main() {
  std::filesystem::create_directory(common::get_data_dir() + std::string("/images/"));
  std::filesystem::create_directory(common::get_data_dir() + std::string("/images/landcover/"));
  std::filesystem::create_directory(common::get_data_dir() + std::string("/images/elevation/"));

  asio::io_context io_context;
  TCPServer tcp_server(io_context);
  asio::thread t(boost::bind(&asio::io_context::run, &io_context));
  SimServer sim_server(tcp_server);
  while (true) {
    sim_server.step();
  }

  return 0;
}