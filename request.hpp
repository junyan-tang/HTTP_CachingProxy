#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include <iostream>
#include <string_view>

namespace http = boost::beast::http;
using namespace boost::asio;
using boost::asio::ip::tcp;

class Request {
protected:
  http::request<http::string_body> req;
  std::string_view requestType;
  std::string_view target;
  int version;
  http::fields headers;
  int id;

public:
  Request(http::request<http::string_body> &m_req)
      : req(m_req),
        requestType(req.method_string().data(), req.method_string().size()),
        target(req.target().data(), req.target().size()),
        version(req.version()),
        headers(req.base()) {}
  std::string_view getRequestType() { return requestType; }
  std::string_view getTarget() { return target; }
  std::string getTargetHost() { return std::string(target.substr(0, target.find(':'))); }
  std::string getTargetPort() { return std::string(target.substr(target.find(':') + 1)); }
  int getVersion() { return version; }
  http::fields getHeaders() { return headers; }
  int getID() { return id; }
  http::request<http::string_body> getRequest() { return req; }
};

#endif