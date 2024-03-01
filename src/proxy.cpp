#include "proxy.hpp"
#include "cache.hpp"
#include "log.hpp"
#include "request.hpp"
#include <boost/beast/http/read.hpp>
#include <iostream>

// The basic implementation of proxy server and client session referenced from
// the examples from asio section in official boost documentation
// https://beta.boost.org/doc/libs/1_82_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

namespace http = boost::beast::http;

static size_t session_id = 1;
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
                  m_target_socket.close();
                  is_forwarding = false;
                  readRequest();
                }
              });
        } else {
          // Handle read error or close connection
          m_target_socket.close();
          is_forwarding = false;
          readRequest();
        }
      });
}

void ClientSession::startForwarding() {
  auto self(shared_from_this());
  doForward(m_socket, m_target_socket, m_buffer_client);
  doForward(m_target_socket, m_socket, m_buffer_target);
}

void ClientSession::processGET(Request &req) {
  std::string_view uri = req.getTarget();
  std::string cache_status = cache.isCacheUsable(uri, session_id);
  http::request<http::string_body> req_body = req.getRequest();

  if (cache_status == "valid") {
    http::response<http::string_body> resp = cache.getCachedPage(uri);
  } else if(cache_status == "revalidation") {
    http::response<http::string_body> resp = cache.getCachedPage(uri);
    std::string eTagValue = getETag(resp);
    std::string lastModified = getLastModified(resp);
    if(eTagValue != ""){
      req_body.set(http::field::if_none_match, eTagValue);
    }
    if(lastModified != ""){
      req_body.set(http::field::if_modified_since, lastModified);
    }
    ClientSession::requestFromServer(req);
  } else {
    logFile << session_id << ": Requesting " << req.getFirstLine() << " from "
            << req.getTargetHost() << std::endl;
    ClientSession::requestFromServer(req);
  }
  Response resp(m_response);
  int respCode = resp.getStatusCode();

  if (respCode == 200) {
    if (resp.isCacheable() != "") {
      logFile << session_id << ": not cacheable because " << resp.isCacheable()
              << std::endl;
    } else {
      cache.addToCache(uri, resp);
      if (cache.checkValidation(resp.getResponse())) {
        logFile << session_id << ": cached, but requires re-validation"
                << std::endl;
      } else if (resp.checkExpireTime() != "") {
        logFile << session_id << ": cached, expires at "
                << resp.checkExpireTime() << std::endl;
      }
    }
  } else if(respCode == 304){
    logFile << session_id << ": Not modified" << std::endl;
  }
}

void ClientSession::requestFromServer(Request &req) {
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
                  std::cerr << "POST/GET Request Send Error: " << ec.message()
                            << std::endl;
                }
              });
        } else {
          // connect failed
          m_response.result(http::status::bad_gateway);
          sendResponse();
        }
      });
}

void ClientSession::processPOST(Request &req) {
  logFile << session_id << ": Requesting " << req.getFirstLine() << " from "
          << req.getTargetHost() << std::endl;
  ClientSession::requestFromServer(req);
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
          // connect successful
          m_response.result(http::status::ok);
          m_response.set(http::field::connection, "keep-alive");
          is_forwarding = true;
          sendResponse();
          startForwarding();
        } else {
          // connect failed
          m_response.result(http::status::bad_gateway);
          sendResponse();
        }
      });
  logFile << session_id << ": Tunnel closed" << std::endl;
}

ClientSession::RequestHandler
ClientSession::getHandler(const std::string_view &requestType) {
  if (requestType == "GET") {
    return &ClientSession::processGET;
  } else if (requestType == "POST") {
    return &ClientSession::processPOST;
  } else if (requestType == "CONNECT") {
    return &ClientSession::processCONNECT;
  } else {
    // handle malformed type
    logFile << session_id << ": Responding "
            << " receive a malformed request" << std::endl;
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
          Request req(m_request);
          std::string ip_from =
              m_socket.remote_endpoint().address().to_string();
          auto receive_time = std::chrono::system_clock::to_time_t(
              std::chrono::system_clock::now());
          logFile << session_id << ": \"" << req.getFirstLine() << "\" from "
                  << ip_from << " @ " << std::ctime(&receive_time);
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
          if (!is_forwarding) {
            readRequest();
          }
        } else {
          std::cerr << "Response Send Error: " << ec.message() << std::endl;
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