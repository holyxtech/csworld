#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H
#include <boost/bind/bind.hpp>
#define ASIO_HAS_BOOST_BIND
#include <asio.hpp>
#include "../readerwriterqueue.h"
#include "types.h"

using asio::ip::tcp;

class TCPConnection;

class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
  typedef std::shared_ptr<TCPConnection> pointer;
  tcp::socket& socket();

  static pointer create(asio::io_context& io_context, int id, moodycamel::ReaderWriterQueue<MessageWithId>& q);

  void write(Message message);
  void start();

private:
  TCPConnection(asio::io_context& io_context, int id, moodycamel::ReaderWriterQueue<MessageWithId>& q);
  void handle_read_header(const ::asio::error_code& error);
  void handle_read_body(const asio::error_code& error, uint32_t body_length);
  void handle_write(); 

  int id_;
  tcp::socket socket_;
  static constexpr int header_length = 4;
  static constexpr int buffer_size = 1048576;
  uint8_t read_buffer_[buffer_size];
  moodycamel::ReaderWriterQueue<MessageWithId>& q_;
};

#endif