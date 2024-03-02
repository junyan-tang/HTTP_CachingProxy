#ifndef __PROXY_HPP__
#define __PROXY_HPP__

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <cstddef>
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
  tcp::socket m_target_socket;
  boost::beast::flat_buffer m_buffer_client;
  boost::beast::flat_buffer m_buffer_target;
  http::request<http::dynamic_body> m_request;
  http::response<http::dynamic_body> m_response;
  size_t m_id;
  bool is_forwarding = false;

  void readRequest();
  void sendResponse();
  void processGET(Request &req);
  void processPOST(Request &req);
  void processCONNECT(Request &req);
  void processMalformed();
  void requestFromServer(Request &req, std::function<void()> callback);
  void tryAddToCache(std::string_view uri);
  void startForwarding();
  void doForward(tcp::socket &source, tcp::socket &target,
                 boost::beast::flat_buffer &buffer);
  void writeErrorLog(const std::string &message);

public:
  explicit ClientSession(tcp::socket socket);
  void startService();
  typedef void (ClientSession::*RequestHandler)(Request &);
  static RequestHandler getHandler(const std::string_view &requestType);
};

// The proxy server listens for incoming client connections
class ProxyServer {
private:
  tcp::acceptor m_acceptor;
  void acceptConnection();

public:
  ProxyServer(boost::asio::io_context &ioContext, short port);
};

#endif