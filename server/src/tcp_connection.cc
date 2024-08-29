#include "tcp_connection.h"

TCPConnection::TCPConnection(asio::io_context& io_context, int id, moodycamel::ReaderWriterQueue<MessageWithId>& q) : socket_(io_context), id_(id), q_(q) {}

tcp::socket& TCPConnection::socket() {
  return socket_;
}

TCPConnection::pointer TCPConnection::create(asio::io_context& io_context, int id, moodycamel::ReaderWriterQueue<MessageWithId>& q) {
  return pointer(new TCPConnection(io_context, id, q));
}

void TCPConnection::start() {
  asio::async_read(
    socket_,
    asio::buffer(read_buffer_, header_length),
    boost::bind(&TCPConnection::handle_read_header, this, asio::placeholders::error));
}

void TCPConnection::write(Message message) {
  asio::error_code ignored_error;
  asio::async_write(
    socket_,
    asio::buffer(message.data(), message.size()),
    boost::bind(&TCPConnection::handle_write, this));
}

void TCPConnection::handle_read_header(const ::asio::error_code& error) {
  if (!error) {
    uint32_t body_length;
    std::memcpy(&body_length, read_buffer_.data(), sizeof(body_length));

    asio::async_read(
      socket_,
      asio::buffer(read_buffer_, body_length),
      boost::bind(&TCPConnection::handle_read_body, this, asio::placeholders::error, body_length));

  }
}

void TCPConnection::handle_read_body(const asio::error_code& error, uint32_t body_length) {
  if (!error) {
    MessageWithId msg_with_id = {Message(body_length), id_};
    std::memcpy(msg_with_id.message.data(), read_buffer_.data(), body_length);
    q_.enqueue(std::move(msg_with_id));

    asio::async_read(
      socket_,
      asio::buffer(read_buffer_, header_length),
      boost::bind(&TCPConnection::handle_read_header, this, asio::placeholders::error));
  }
}

void TCPConnection::handle_write() {}
