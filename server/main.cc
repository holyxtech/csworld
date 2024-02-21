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

  /*   WorldGenerator wg;
    int x = 3950000 / Chunk::sz_x;
    int z = -1800000 / Chunk::sz_z;
    // int x = 0, z = 0;
    Chunk chunk(x, 0, z);
    wg.get_chunk_terrain_data(chunk);
    x = 3950064 / Chunk::sz_x;
    z = -1800000 / Chunk::sz_z;
    // int x = 0, z = 0;
    Chunk chunk_2(x, 0, z);
    wg.get_chunk_terrain_data(chunk_2);
   */
  return 0;
}