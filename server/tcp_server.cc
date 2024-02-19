#include "tcp_server.h"

TCPServer::TCPServer(asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), 7331)), q_(100) {
  start_accept();
  std::cout << "Started listening on port 7331" << std::endl;
}
void TCPServer::write(MessageWithId msg_with_id) {
  auto connection = connections_[msg_with_id.id];
  connection->write(msg_with_id.message);
}

moodycamel::ReaderWriterQueue<MessageWithId>& TCPServer::get_queue() {
  return q_;
}

void TCPServer::start_accept() {
  auto new_connection = TCPConnection::create(io_context_, connections_.size(), q_);
  connections_.push_back(new_connection);
  acceptor_.async_accept(
    new_connection->socket(),
    boost::bind(
      &TCPServer::handle_accept, this, new_connection,
      asio::placeholders::error));
}

void TCPServer::handle_accept(
  TCPConnection::pointer new_connection,
  const asio::error_code& error) {

  if (!error) {
    new_connection->start();
  }

  start_accept();
}
