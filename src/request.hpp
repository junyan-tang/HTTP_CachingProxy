#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <regex>
#include <string_view>

namespace http = boost::beast::http;
using namespace boost::asio;
using boost::asio::ip::tcp;

class Request {
protected:
  http::request<http::string_body> req;
  std::string_view requestType;
  std::string_view target;
  http::fields headers;
  int version;

public:
  Request(http::request<http::dynamic_body> &m_req)
      : req(m_req),
        requestType(req.method_string().data(), req.method_string().size()),
        target(req.target().data(), req.target().size()), headers(req.base()),
        version(req.version()) {}
  std::string_view getRequestType() { return requestType; }
  std::string_view getTarget() { return target; }
  std::string getTargetHost() {
    std::string host = req["Host"].to_string();
    auto pos = host.find(':');
    if (pos != std::string::npos) {
      return host.substr(0, pos);
    } else {
      return host;
    }
  }
  std::string getTargetPort() {
    std::string port;
    if (requestType == "CONNECT") {
      port = "443";
    } else {
      port = "80";
    }
    std::string host = req["Host"].to_string();
    auto pos = host.find(':');
    if (pos != std::string::npos) {
      return host.substr(pos + 1);
    } else {
      return port;
    }
  }
  int getVersion() { return version; }
  http::fields getHeaders() { return headers; }
  http::request<http::string_body> getRequest() { return req; }
  std::string getFirstLine() {
    std::ostringstream ss;
    ss << requestType << " " << target << " HTTP/" << version / 10 << "."
       << version % 10;
    return ss.str();
  }
};

#endif