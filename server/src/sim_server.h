#ifndef SIM_SERVER_H
#define SIM_SERVER_H
#include "tcp_server.h"
#include "types.h"
#include "world_generator.h"

class SimServer {
public:
  SimServer(TCPServer& tcp_server);
  void step();

private:
  TCPServer& tcp_server_;
  WorldGenerator world_generator_;
};
#endif