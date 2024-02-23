#include "proxy.hpp"
#include "log.hpp"
#include "request.hpp"
#include <iostream>

// The basic implementation of proxy server and client session referenced from
// the examples from asio section in official boost documentation
// https://beta.boost.org/doc/libs/1_82_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

namespace http = boost::beast::http;
ClientSession::ClientSession(tcp::socket socket)
    : m_socket(std::move(socket)) {}

void ClientSession::startService() { readRequest(); }

// Asynchronously read data from the client
void ClientSession::readRequest() {
  auto self(shared_from_this());
  
  http::async_read(m_socket, m_buffer, m_request,
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          // Placeholder for request processing logic
          if (length > 0) {
            std::cout << "Request Method: " << m_request.method_string()
                      << std::endl;
          }
          // Simply echo the request back
          sendResponse();
        }
      });
}

// Asynchronously send data back to the client
void ClientSession::sendResponse() {
  auto self(shared_from_this());
  http::async_write(
      m_socket, m_response,
      [this, self](boost::system::error_code ec, std::size_t length) mutable{
        if (!ec) {
          // Read the next request
          // std::cout << "Response sent" << std::endl;
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
  m_acceptor.async_accept(
      [this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
          // Create a session for each connection
          std::make_shared<ClientSession>(std::move(socket))->startService();
        }
        // Wait for another connection
        acceptConnection();
      });
}

int main() {
  try {
    boost::asio::io_context ioContext;
    ProxyServer server(ioContext, 12345);

    ioContext.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}