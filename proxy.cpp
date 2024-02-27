#include "proxy.hpp"
#include "cache.hpp"
// #include "log.hpp"
#include "request.hpp"
#include <boost/beast/http/read.hpp>
#include <iostream>

// The basic implementation of proxy server and client session referenced from
// the examples from asio section in official boost documentation
// https://beta.boost.org/doc/libs/1_82_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

namespace http = boost::beast::http;

static size_t session_id = 0;
static int request_id = 0;
Cache cache(100);

void ClientSession::doForward(tcp::socket &source, tcp::socket &target,
                              boost::beast::flat_buffer &buffer) {
  auto self(shared_from_this());
  source.async_read_some(
      buffer.prepare(4096), // Adjust the size as needed
      [this, self, &source, &target, &buffer](boost::system::error_code ec,
                                              std::size_t bytes_transferred) {
        if (!ec) {
          buffer.commit(bytes_transferred);
          boost::asio::async_write(
              target, buffer.data(),
              [this, self, &source, &target,
               &buffer](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                  buffer.consume(
                      buffer.size()); // Clear the buffer after writing
                  doForward(source, target,
                            buffer); // Continue the forwarding loop
                } else {
                  // Handle write error or close connection
                  source.close();
                  target.close();
                }
              });
        } else {
          // Handle read error or close connection
          source.close();
          target.close();
        }
      });
}

void ClientSession::startForwarding() {
  auto self(shared_from_this());
  doForward(m_socket, m_target_socket, m_buffer_client);
  doForward(m_target_socket, m_socket, m_buffer_target);
}

void ClientSession::processGET(Request &req) {
  // std::string_view uri = req.getTarget();
  // // check if it in cache and can be use
  // if (cache.isCacheUsable(uri, req.getID())){
  //   http::response<http::string_body> resp = cache.getCachedPage(uri);
  // }
  // else{
  //   logFile << req.getID() << ": Requesting " << req << " from " << <<
  //   std::endl;
  //   //send to original server and recieve
  //   logFile << req.getID() << ": Received " << resp << " from " << <<
  //   std::endl;
  // }

  // // if receive 200-ok
  // if (false) {
  //   // not cacheable
  //   if (false) {
  //     // add reason
  //     logFile << req.getID() << ": not cacheable because "
  //             << "reason holder" << std::endl;
  //   } else {
  //     // add to cache
  //     //  if need revalidation
  //     if (false) {
  //       logFile << req.getID() << ": cached, but requires re-validation"
  //               << std::endl;
  //     }
  //     // if it has expire time
  //     else if (false) {
  //       logFile << req.getID() << ": cached, expires at " << time <<
  //       std::endl;
  //     }
  //   }
  // }
  ClientSession::processPOST(req);
}

void ClientSession::processPOST(Request &req) {
  // directly forward to original servers
  // ID: Received "RESPONSE" from SERVER
  // logFile << req.getID() << ": Requesting " << req << " from " << <<
  // std::endl;
  std::string host = req.getTargetHost();
  std::string port = req.getTargetPort();
  std::cout << "Host: " << host << " Port: " << port << std::endl;

  boost::asio::io_context &ioContext =
      static_cast<boost::asio::io_context &>(m_socket.get_executor().context());

  if (m_target_socket.is_open()) {
    m_target_socket.close();
  }
  // Create a new socket for the connection to the target host
  m_target_socket = tcp::socket(ioContext);

  // Resolve the target host name
  tcp::resolver resolver(ioContext);
  auto endpoints = resolver.resolve(host, port);

  auto self(shared_from_this());
  
  boost::asio::async_connect(
      m_target_socket, endpoints,
      [this, self](boost::system::error_code ec, const tcp::endpoint &) {
        if (!ec) {
          http::async_write(
              m_target_socket, m_request,
              [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                  http::async_read(m_target_socket, m_buffer_target, m_response,
                                   [this, self](boost::system::error_code ec,
                                                std::size_t length) {
                                     if (!ec) {
                                       sendResponse();
                                     }
                                   });
                } else {
                  std::cout << "Post Request Send Error: " << ec.message()
                            << std::endl;
                }
              }

          );
        } else {
          // connect failed
          m_response.result(http::status::bad_gateway);
          sendResponse();
        }
      });
}

void ClientSession::processCONNECT(Request &req) {
  std::string host = req.getTargetHost();
  std::string port = req.getTargetPort();

  boost::asio::io_context &ioContext =
      static_cast<boost::asio::io_context &>(m_socket.get_executor().context());

  if (m_target_socket.is_open()) {
    m_target_socket.close();
  }
  // Create a new socket for the connection to the target host
  m_target_socket = tcp::socket(ioContext);

  // Resolve the target host name
  tcp::resolver resolver(ioContext);
  auto endpoints = resolver.resolve(host, port);

  // Asynchronously connect to the target host
  auto self(shared_from_this());
  boost::asio::async_connect(
      m_target_socket, endpoints,
      [this, self](boost::system::error_code ec, const tcp::endpoint &) {
        if (!ec) {
          // boost::asio::socket_base::keep_alive option(true);
          // m_target_socket.set_option(option);
          // connect successful
          m_response.result(http::status::ok);
          m_response.set(http::field::connection, "keep-alive");
          sendResponse();
          startForwarding();
        } else {
          // connect failed
          m_response.result(http::status::bad_gateway);
          sendResponse();
        }
      });
}

ClientSession::RequestHandler
ClientSession::getHandler(const std::string_view &requestType) {
  // assign it a unique id (ID), and print the ID, time received (TIME), IP
  // address the request was received from (IPFROM) and the HTTP request line
  // (REQUEST) of the request in the following format: ID: "REQUEST" from IPFROM
  // @ TIME
  if (requestType == "GET") {
    return &ClientSession::processGET;
  } else if (requestType == "POST") {
    return &ClientSession::processPOST;
  } else if (requestType == "CONNECT") {
    return &ClientSession::processCONNECT;
  } else {
    // Handle unknown request type
    // ID: Responding "RESPONSE" receive a malformed request
    return nullptr;
  }
}

ClientSession::ClientSession(tcp::socket socket)
    : m_socket(std::move(socket)), m_target_socket(m_socket.get_executor()),
      m_id(session_id) {
  session_id += 1;
}

void ClientSession::startService() { readRequest(); }

// Asynchronously read data from the client
void ClientSession::readRequest() {
  auto self(shared_from_this());
  http::async_read(
      m_socket, m_buffer_client, m_request,
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          std::cout << "Client session " << m_id
                    << " received a request:" << std::endl;
          std::cout << m_request << std::endl;
          Request req(m_request);
          ClientSession::RequestHandler handler =
              ClientSession::getHandler(req.getRequestType());
          if (handler) {
            (this->*handler)(req);
          }
          // TODO: Handle unknown request type
        }
      });
}

// Asynchronously send data back to the client
void ClientSession::sendResponse() {
  auto self(shared_from_this());
  http::async_write(
      m_socket, m_response,
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          // Read the next request
          std::cout << "Response sent" << std::endl;
          std::cout << m_response << std::endl;
          readRequest();
        } else {
          std::cout << "Response Send Error: " << ec.message() << std::endl;
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