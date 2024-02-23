#ifndef PROXY_HPP 
#define PROXY_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <memory>
#include <vector>

using boost::asio::ip::tcp;
namespace http = boost::beast::http;

// A session represents a single connection from a client
class ClientSession : public std::enable_shared_from_this<ClientSession> {
private:
  tcp::socket m_socket;
  boost::beast::flat_buffer m_buffer;
  http::request<http::string_body> m_request;
  http::response<http::string_body> m_response;

  void readRequest();
  void sendResponse();

public:
  explicit ClientSession(tcp::socket socket);
  void startService();
};

// The proxy server listens for incoming client connections
class ProxyServer {
private:
  tcp::acceptor m_acceptor;

  void acceptConnection();

public:
  ProxyServer(boost::asio::io_service &ioContext, short port);
};

#endif