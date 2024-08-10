#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <optional>
#include <sqlite3.h>
#include "chunk.h"

class DbManager {
public:
  DbManager();
  ~DbManager();
  void save_chunk(const Chunk& chunk);
  std::optional<Chunk> load_chunk_if_exists(const Location& loc);

private:
  sqlite3* db_;
};

#endif