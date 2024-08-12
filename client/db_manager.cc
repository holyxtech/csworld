#include "db_manager.h"

#include <filesystem>
#include <string>
#include <vector>
#include "config.h"

DbManager::DbManager() {
  auto path = APPLICATION_DATA_DIR + std::string("db.sqlite");
  bool initialize_db = false;
  std::filesystem::path fs_path(path);
  if (!std::filesystem::exists(fs_path)) {
    initialize_db = true;
  }

  int failure = sqlite3_open(path.c_str(), &db_);
  if (failure) {
    std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << std::endl;
    throw std::runtime_error("Failed to initialize DbManager");
  }
  if (initialize_db) {
    std::string sql =
      "create table Chunk("
      "\tx integer not null,"
      "\ty integer not null,"
      "\tz integer not null,"
      "\tdata blob,"
      "\tprimary key (x,y,z)"
      ");";
    char* err_msg;
    int failure = sqlite3_exec(db_, sql.c_str(), NULL, 0, &err_msg);
    if (failure) {
      std::cerr << "Failed to create table: " << err_msg << std::endl;
      sqlite3_free(err_msg);
      throw std::runtime_error("Failed to initialize DbManager");
    }
  }
}

DbManager::~DbManager() {
  sqlite3_close(db_);
}

std::optional<Chunk> DbManager::load_chunk_if_exists(const Location& loc) {
  sqlite3_stmt* stmt;
  std::string sql = "select data from Chunk where x = ? and y = ? and z = ?";
  sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
  sqlite3_bind_int(stmt, 1, loc[0]);
  sqlite3_bind_int(stmt, 2, loc[1]);
  sqlite3_bind_int(stmt, 3, loc[2]);
  int rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    const unsigned char* data = static_cast<const unsigned char*>(sqlite3_column_blob(stmt, 0));
    int data_size = sqlite3_column_bytes(stmt, 0);
    auto chunk = Chunk(loc, data, data_size);
    sqlite3_finalize(stmt);
    return chunk;
  } else {
    sqlite3_finalize(stmt);
    return {};
  }
}

void DbManager::save_chunk(const Chunk& chunk) {
  std::vector<std::uint32_t> runs;
  auto& loc = chunk.get_location();
  runs.reserve(Chunk::sz); // worst case
  auto last_voxel = chunk.get_voxel(0, 0, 0);
  std::uint32_t run_length = 0;
  for (int y = 0; y < Chunk::sz_y; ++y) {
    for (int x = 0; x < Chunk::sz_x; ++x) {
      for (int z = 0; z < Chunk::sz_z; ++z) {
        auto voxel = chunk.get_voxel(x, y, z);
        if (voxel == last_voxel) {
          ++run_length;
        } else {
          std::uint32_t run = (static_cast<std::uint32_t>(last_voxel) << 16) | run_length;
          runs.push_back(run);
          last_voxel = voxel;
          run_length = 1;
        }
      }
    }
  }
  std::string sql = "insert or replace into Chunk(x,y,z,data) values(?,?,?,?);";
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
  sqlite3_bind_int(stmt, 1, loc[0]);
  sqlite3_bind_int(stmt, 2, loc[1]);
  sqlite3_bind_int(stmt, 3, loc[2]);
  int runs_size = sizeof(std::uint32_t) * runs.size();
  sqlite3_bind_blob(stmt, 4, runs.data(), runs_size, SQLITE_STATIC);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
}
