#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "tcp_connection.h"

class TCPServer {
public:
  TCPServer(asio::io_context& io_context);
  void write(const MessageWithId& msg_with_id);
  moodycamel::ReaderWriterQueue<MessageWithId>& get_queue();

private:
  void start_accept();

  void handle_accept(TCPConnection::pointer new_connection, const asio::error_code& error);

  std::vector<TCPConnection::pointer> connections_;
  asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  moodycamel::ReaderWriterQueue<MessageWithId> q_;
};

#endif