#ifndef SIM_SERVER_H
#define SIM_SERVER_H
#include "../fbs/cs_generated.h"
#include "tcp_server.h"
#include "types.h"

class SimServer {
public:
  SimServer(TCPServer& tcp_server);
  void step();

private:
  TCPServer& tcp_server_;
};
#endif