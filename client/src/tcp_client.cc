#include "tcp_client.h"

TCPClient::TCPClient(asio::io_context& io_context)
    : io_context_{io_context}, socket_{io_context} {
  tcp::resolver resolver(io_context_);
  auto* host = "127.0.0.1";
  auto endpoints = resolver.resolve(host, "7331");
  asio::connect(socket_, endpoints);
  handle_connect(asio::error_code());
}

void TCPClient::write(const Message& message) {
  asio::async_write(
    socket_,
    asio::buffer(message.data(), message.size()),
    boost::bind(&TCPClient::handle_write, this));
}

void TCPClient::handle_write() {}

void TCPClient::handle_connect(const asio::error_code& error) {
  if (error)
    throw std::runtime_error(error.message());
  std::cout << "connection established" << std::endl;

  asio::async_read(
    socket_,
    asio::buffer(read_buffer_, header_length),
    boost::bind(
      &TCPClient::handle_read_header, this,
      asio::placeholders::error));
}

moodycamel::ReaderWriterQueue<Message>& TCPClient::get_queue() {
  return q_;
}

void TCPClient::handle_read_header(const asio::error_code& error) {
  std::uint32_t body_length;
  std::memcpy(&body_length, read_buffer_.data(), sizeof(body_length));

  asio::async_read(
    socket_,
    asio::buffer(read_buffer_, body_length),
    boost::bind(
      &TCPClient::handle_read_body, this,
      asio::placeholders::error, body_length));
}

void TCPClient::handle_read_body(const asio::error_code& error, std::uint32_t body_length) {

  Message message(body_length);
  std::memcpy(message.data(), read_buffer_.data(), body_length);
  q_.enqueue(std::move(message));

  asio::async_read(
    socket_,
    asio::buffer(read_buffer_, header_length),
    boost::bind(
      &TCPClient::handle_read_header, this,
      asio::placeholders::error));
}
