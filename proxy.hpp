#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <vector>

using boost::asio::ip::tcp;

// The basic implementation of proxy server and client session referenced from
// the examples from asio section in official boost documentation
// https://beta.boost.org/doc/libs/1_82_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

// A session represents a single connection from a client
class ClientSession : public std::enable_shared_from_this<ClientSession> {
private:
  tcp::socket m_socket;
  std::array<char, 1024> m_buffer;

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