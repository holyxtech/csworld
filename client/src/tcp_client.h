#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#ifdef _WIN32
#include <SDKDDKVer.h>
#endif
#include "common.h"
#include <array>
#include <asio.hpp>
#include <boost/bind/bind.hpp>
#include "readerwriterqueue.h"
#include "types.h"

using asio::ip::tcp;

class TCPClient {

public:
  TCPClient(asio::io_context& io_context);
  void write(const Message& message);
  moodycamel::ReaderWriterQueue<Message>& get_queue();

private:
  void handle_connect(const asio::error_code& error);
  void handle_read_header(const asio::error_code& error);
  void handle_read_body(const asio::error_code& error, std::uint32_t body_length);
  void handle_write();

  asio::io_context& io_context_;
  tcp::socket socket_;
  std::array<std::uint8_t, common::max_msg_buffer_size> read_buffer_;
  static constexpr int header_length = 4;
  moodycamel::ReaderWriterQueue<Message> q_;
};

#endif