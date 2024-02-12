#include "tcp_client.h"
#include <iostream>

TCPClient::TCPClient(asio::io_context& io_context)
    : io_context_(io_context), socket_(io_context) {
  tcp::resolver resolver(io_context_);
  auto* host = "localhost";
  auto endpoints = resolver.resolve(host, "7331");

  asio::async_connect(
    socket_,
    endpoints,
    boost::bind(
      &TCPClient::handle_connect, this,
      asio::placeholders::error));
}

void TCPClient::write(Message&& message) {
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
    asio::buffer(read_buffer_, header_length_),
    boost::bind(
      &TCPClient::handle_read_header, this,
      asio::placeholders::error));
}

moodycamel::ReaderWriterQueue<Message>& TCPClient::get_queue() {
  return q_;
}

void TCPClient::handle_read_header(const asio::error_code& error) {
  uint32_t body_length;
  std::memcpy(&body_length, read_buffer_, sizeof(body_length));

  asio::async_read(
    socket_,
    asio::buffer(read_buffer_, body_length),
    boost::bind(
      &TCPClient::handle_read_body, this,
      asio::placeholders::error, body_length));
}

void TCPClient::handle_read_body(const asio::error_code& error, uint32_t body_length) {

  Message message(body_length);
  std::memcpy(message.data(), read_buffer_, body_length);
  q_.enqueue(std::move(message));

  asio::async_read(
    socket_,
    asio::buffer(read_buffer_, buffer_size_),
    boost::bind(
      &TCPClient::handle_read_header, this,
      asio::placeholders::error));
}
