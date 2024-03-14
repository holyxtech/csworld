#include "sim_server.h"
#include "../fbs/common_generated.h"
#include "../fbs/request_generated.h"
#include "../fbs/update_generated.h"

SimServer::SimServer(TCPServer& tcp_server) : tcp_server_(tcp_server) {}

void SimServer::step() {
  MessageWithId msg_with_id;
  auto& q = tcp_server_.get_queue();
  bool success = q.try_dequeue(msg_with_id);
  while (success) {
    auto id = msg_with_id.id;
    auto& message = msg_with_id.message;

    auto* request = fbs_request::GetRequest(message.data());

    auto* sections = request->sections();

    // construct new update
    flatbuffers::FlatBufferBuilder builder(1048576);
    std::vector<flatbuffers::Offset<fbs_update::Section>> returning_sections;

    for (int i = 0; i < sections->size(); ++i) {
      auto* loc = sections->Get(i);
      auto x = loc->x(), y = loc->y();
      auto sec = world_generator_.get_section(Location2D{x, y});
      auto landcover = builder.CreateVector(reinterpret_cast<uint8_t*>(sec.landcover.data()), sec.landcover.size());
      auto section = fbs_update::CreateSection(builder, loc, sec.elevation, landcover);
      returning_sections.push_back(std::move(section));
    }

    auto returned_region = fbs_update::CreateRegionUpdate(builder, builder.CreateVector(returning_sections));
    auto update_kind = fbs_update::UpdateKind_Region;
    auto returned_update = fbs_update::CreateUpdate(builder, fbs_update::UpdateKind_Region, returned_region.Union());
    FinishSizePrefixedUpdateBuffer(builder, returned_update);

    const auto* buffer_pointer = builder.GetBufferPointer();
    const auto buffer_size = builder.GetSize();

    Message returned_message(buffer_size);
    std::memcpy(returned_message.data(), buffer_pointer, buffer_size);
    tcp_server_.write(MessageWithId{returned_message, id});
    success = q.try_dequeue(msg_with_id);
  }
}
