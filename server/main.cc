#include <array>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <boost/bind/bind.hpp>

#define ASIO_HAS_BOOST_BIND
#include <asio.hpp>
#include "../fbs/cs_generated.h"
#include "../readerwriterqueue.h"
#include "sim_server.h"
#include "tcp_connection.h"
#include "tcp_server.h"

using asio::ip::tcp;

class TCPServer;

using Message = std::vector<uint8_t>;
struct MessageWithId {
  Message message;
  int id;
};

moodycamel::ReaderWriterQueue<MessageWithId> q(100);

std::string make_daytime_string() {
  using namespace std;
  time_t now = time(0);
  return ctime(&now);
}

class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
  typedef std::shared_ptr<TCPConnection> pointer;

  tcp::socket& socket() {
    return socket_;
  }

  static pointer create(asio::io_context& io_context, int id) {
    return pointer(new TCPConnection(io_context, id));
  }

  void write(Message message) {

    asio::error_code ignored_error;
    asio::async_write(
      socket_,
      asio::buffer(message.data(), message.size()),
      boost::bind(&TCPConnection::handle_write, this));
  }

  void start() {
    asio::async_read(
      socket_,
      asio::buffer(read_buffer_, header_length),
      boost::bind(&TCPConnection::handle_read_header, this, asio::placeholders::error));
  }

private:
  TCPConnection(asio::io_context& io_context, int id)
      : socket_(io_context), id_(id) {
  }
  void handle_read_header(const ::asio::error_code& error) {
    if (!error) {

      uint32_t body_length;
      std::memcpy(&body_length, read_buffer_, sizeof(body_length));

      asio::async_read(
        socket_,
        asio::buffer(read_buffer_, body_length),
        boost::bind(&TCPConnection::handle_read_body, this, asio::placeholders::error, body_length));
    }
  }

  void handle_read_body(const asio::error_code& error, uint32_t body_length) {
    if (!error) {
      MessageWithId msg_with_id = {Message(body_length), id_};
      std::memcpy(msg_with_id.message.data(), read_buffer_, body_length);
      q.enqueue(std::move(msg_with_id));

      asio::async_read(
        socket_,
        asio::buffer(read_buffer_, buffer_size_),
        boost::bind(&TCPConnection::handle_read_header, this, asio::placeholders::error));
    }
  }

  void handle_write() {}

  int id_;
  tcp::socket socket_;
  static constexpr int header_length = 4;
  static constexpr int buffer_size_ = 8192;
  uint8_t read_buffer_[buffer_size_];
};

class TCPServer {
public:
  TCPServer(asio::io_context& io_context)
      : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), 7331)) {
    start_accept();
    std::cout << "Started listening on port 7331" << std::endl;
  }
  void write(MessageWithId msg_with_id) {
    auto connection = connections_[msg_with_id.id];
    connection->write(msg_with_id.message);
  }

private:
  void start_accept() {
    auto new_connection = TCPConnection::create(io_context_, connections_.size());
    connections_.push_back(new_connection);
    acceptor_.async_accept(
      new_connection->socket(),
      boost::bind(
        &TCPServer::handle_accept, this, new_connection,
        asio::placeholders::error));
  }

  void handle_accept(
    TCPConnection::pointer new_connection,
    const asio::error_code& error) {

    if (!error) {
      new_connection->start();
    }

    start_accept();
  }

  std::vector<TCPConnection::pointer> connections_;
  asio::io_context& io_context_;
  tcp::acceptor acceptor_;
};

class SimServer {
public:
  SimServer(TCPServer& tcp_server) : tcp_server_(tcp_server) {
  }
  void step() {
    MessageWithId msg_with_id;
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
        flatbuffers::FlatBufferBuilder builder(1024);
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

private:
  TCPServer& tcp_server_;
};

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