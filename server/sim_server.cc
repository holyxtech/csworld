#include "sim_server.h"

SimServer::SimServer(TCPServer& tcp_server) : tcp_server_(tcp_server) {}

void SimServer::step() {
  MessageWithId msg_with_id;
  auto& q = tcp_server_.get_queue();
  bool success = q.try_dequeue(msg_with_id);
  while (success) {
    auto id = msg_with_id.id;
    auto& message = msg_with_id.message;

    auto* update = fbs::GetUpdate(message.data());

    switch (update->kind_type()) {
    case fbs::UpdateKind_Region:
      auto* region = update->kind_as_Region();
      auto* chunks = region->chunks();

      // construct new update
      flatbuffers::FlatBufferBuilder builder(1048576);
      std::vector<flatbuffers::Offset<fbs::Chunk>> returned_chunks;

      for (int i = 0; i < chunks->size(); ++i) {
        auto* loc = chunks->Get(i)->location();
        auto x = loc->x(), y = loc->y(), z = loc->z();

        auto chunk = fbs::CreateChunk(builder, loc);
        returned_chunks.push_back(std::move(chunk));
      }

      auto returned_region = CreateRegionUpdate(builder, builder.CreateVector(returned_chunks));
      auto update_kind = fbs::UpdateKind_Region;
      auto returned_update = CreateUpdate(builder, fbs::UpdateKind_Region, returned_region.Union());
      FinishSizePrefixedUpdateBuffer(builder, returned_update);

      const auto* buffer_pointer = builder.GetBufferPointer();
      const auto buffer_size = builder.GetSize();

      Message returned_message(buffer_size);
      std::memcpy(returned_message.data(), buffer_pointer, buffer_size);
      tcp_server_.write(MessageWithId{returned_message, id});
      break;
    }
    success = q.try_dequeue(msg_with_id);
  }
}
