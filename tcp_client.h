#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <boost/bind/bind.hpp>
#define ASIO_HAS_BOOST_BIND
#include <asio.hpp>
#include "readerwriterqueue.h"
#include "types.h"

using asio::ip::tcp;

class TCPClient {

public:
  TCPClient(asio::io_context& io_context);
  void write(Message&& message);
  moodycamel::ReaderWriterQueue<Message>& get_queue();

private:
  void handle_connect(const asio::error_code& error);
  void handle_read_header(const asio::error_code& error);
  void handle_read_body(const asio::error_code& error, uint32_t body_length);
  void handle_write();

  asio::io_context& io_context_;
  tcp::socket socket_;
  static constexpr int buffer_size_ = 1048576;
  uint8_t read_buffer_[buffer_size_];
  static constexpr int header_length_ = 4;
  moodycamel::ReaderWriterQueue<Message> q_;
};

#endif