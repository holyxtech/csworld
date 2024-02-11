#ifndef SIM_SERVER_H
#define SIM_SERVER_H

#include <queue>
#include "player.h"
#include "region.h"
#include "world_generator.h"

class SimServer {
public:
  SimServer();
  void start();
  void queue_action(int action);
  const Region& get_region() const;
private:
  void step();
  Region region_;

  std::vector<Player> players_;
  std::queue<int> queued_actions_;
};

#endif