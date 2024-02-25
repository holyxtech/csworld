#include <array>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <boost/bind/bind.hpp>

#define ASIO_HAS_BOOST_BIND
#include <asio.hpp>
#include "../readerwriterqueue.h"
#include "chunk.h"
#include "sim_server.h"
#include "tcp_server.h"
#include "world_generator.h"

int main() {

  asio::io_context io_context;
  TCPServer tcp_server(io_context);
  asio::thread t(boost::bind(&asio::io_context::run, &io_context));
  SimServer sim_server(tcp_server);
  while (true) {
    sim_server.step();
  }

  return 0;
}