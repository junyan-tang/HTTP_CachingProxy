#ifndef __PROXY_HPP__
#define __PROXY_HPP__

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <vector>

#include "request.hpp"

using boost::asio::ip::tcp;
namespace http = boost::beast::http;

// The basic implementation of proxy server and client session referenced from
// the examples from asio section in official boost documentation
// https://beta.boost.org/doc/libs/1_82_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

// A session represents a single connection from a client
class ClientSession : public std::enable_shared_from_this<ClientSession> {
private:
  tcp::socket m_socket;
  boost::beast::flat_buffer m_buffer;
  http::request<http::string_body> m_request;
  http::response<http::string_body> m_response;

  void readRequest();
  void sendResponse();
  void processGET(Request& req);
  void processPOST(Request& req);
  void processCONNECT(Request& req);

public:
  explicit ClientSession(tcp::socket socket);
  void startService();
  typedef void (ClientSession::*RequestHandler)(Request&);
  static RequestHandler getHandler(const std::string_view& requestType);
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