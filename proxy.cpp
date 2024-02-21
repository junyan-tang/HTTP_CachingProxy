#include "proxy.hpp"

ClientSession::ClientSession(tcp::socket socket)
    : m_socket(std::move(socket)) {}

void ClientSession::run() { readRequest(); }

// Asynchronously read data from the client
void ClientSession::readRequest() {
  auto self(shared_from_this());
  m_socket.async_read_some(
      boost::asio::buffer(m_buffer),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          // Placeholder for request processing logic
          std::cout << "Request: " << std::string(m_buffer.data(), length)
                    << std::endl;
          // Simply echo the request back
          sendResponse();
        }
      });
}

// Asynchronously send data back to the client
void ClientSession::sendResponse() {
  auto self(shared_from_this());
  boost::asio::async_write(
      m_socket, boost::asio::buffer(m_buffer),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          // Read the next request
          readRequest();
        }
      });
}

ProxyServer::ProxyServer(boost::asio::io_context &ioContext, short port)
    : m_acceptor(ioContext, tcp::endpoint(tcp::v4(), port)) {
  acceptConnection();
}

// Asynchronously accept incoming connections
void ProxyServer::acceptConnection() {
  m_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
    if (!ec) {
      // Create a session for each connection
      std::make_shared<ClientSession>(std::move(socket))->run();
    }
    // Wait for another connection
    acceptConnection();
  });
}
