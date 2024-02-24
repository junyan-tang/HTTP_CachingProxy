#include "proxy.hpp"
#include "log.hpp"
#include "request.hpp"
#include <iostream>

// The basic implementation of proxy server and client session referenced from
// the examples from asio section in official boost documentation
// https://beta.boost.org/doc/libs/1_82_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

namespace http = boost::beast::http;

void ClientSession::processGET(Request &req) {
  //check if it in cache and can be use
  return;
  //else request to the original server
  //ID: Requesting "REQUEST" from SERVER
  //ID: Received "RESPONSE" from SERVER

  //if receive 200-ok
  if(false){
    // not cacheable
    if(false){
      //add reason
      logFile << req.getID() << ": not cacheable because " << "reason holder" << std::endl;
    }
    else{
      //add to cache
      // if need revalidation
      if(false){
        logFile << req.getID() << ": cached, but requires re-validation" << std::endl;
      }
      // if it has expire time
      else if(false){
        logFile << req.getID() << ": cached, expires at " << time << std::endl;
      }
    }

  }
}
void ClientSession::processPOST(Request &req) {
  //directly forward to original server
  //ID: Requesting "REQUEST" from SERVER
  //ID: Received "RESPONSE" from SERVER
}

void ClientSession::processCONNECT(Request &req) {
  std::string host = req.getTargetHost();
  std::string port = req.getTargetPort();

  boost::asio::io_context &ioContext =
      static_cast<boost::asio::io_context &>(m_socket.get_executor().context());
  // Create a new socket for the connection to the target host
  tcp::socket target_socket(ioContext);

  // Resolve the target host name
  tcp::resolver resolver(ioContext);
  auto endpoints = resolver.resolve(host, port);

  // Asynchronously connect to the target host
  auto self(shared_from_this());
  boost::asio::async_connect(
      target_socket, endpoints,
      [this, self, target_socket = std::move(target_socket)](
          boost::system::error_code ec, const tcp::endpoint &) {
        if (!ec) {
          // connect successful
          m_response.result(http::status::ok);
          m_response.set(http::field::connection, "keep-alive");
          sendResponse();
          // TODO: Do we need tunneling here?
          // If tunneling is required, no need to care about other types of
          // requests, just forwarding? Otherwise, we need to handle other types
          // of requests, and we should remember the connection (socket) to the
          // target host, which may use std::unordered_map to record the
          // mapping.
        } else {
          // connect failed
          m_response.result(http::status::bad_gateway);
          sendResponse();
        }
      });
}

ClientSession::RequestHandler
ClientSession::getHandler(const std::string_view &requestType) {
 //assign it a unique id (ID), and print the ID, time received (TIME), IP address the request was received from (IPFROM) and the
 //HTTP request line (REQUEST) of the request in the following format:
 //ID: "REQUEST" from IPFROM @ TIME
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
    : m_socket(std::move(socket)) {}

void ClientSession::startService() { readRequest(); }

// Asynchronously read data from the client
void ClientSession::readRequest() {
  auto self(shared_from_this());
  http::async_read(
      m_socket, m_buffer, m_request,
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          std::cout << "Request method: " << m_request.method_string()
                    << std::endl;
          std::cout << "Request target: " << m_request.target() << std::endl;
          Request req(m_request);
          ClientSession::RequestHandler handler =
              ClientSession::getHandler(req.getRequestType());
          if (handler) {
            (this->*handler)(req);
            sendResponse();
          }
          // TODO: Handle unknown request type
          sendResponse();
        }
      });
}

// Asynchronously send data back to the client
void ClientSession::sendResponse() {
  auto self(shared_from_this());
  http::async_write(
      m_socket, m_response,
      [this, self](boost::system::error_code ec, std::size_t length) mutable {
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