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
  int version;
  http::fields headers;
  int id;

  std::pair<std::string, std::string> parse_url(const std::string &url) {
    std::regex url_pattern(
        R"(^(http:\/\/|https:\/\/)?([^\/:]+)(:\d+)?(\/.*)?$)");
    std::smatch url_match_result;

    std::string host = "";
    std::string port = "";

    if (std::regex_match(url, url_match_result, url_pattern)) {
      // Extract host
      if (url_match_result[2].matched) {
        host = url_match_result[2];
      }
      // Extract port (if specified)
      if (url_match_result[3].matched) {
        port = url_match_result[3].str().substr(1); // Remove the leading colon
      } else if (url_match_result[1].matched) {
        // Assign default port based on the scheme
        std::string scheme = url_match_result[1].str();
        port = (scheme == "http://")    ? "80"
               : (scheme == "https://") ? "443"
                                        : "";
      }
    }

    return {host, port};
  }

public:
  Request(http::request<http::string_body> &m_req)
      : req(m_req),
        requestType(req.method_string().data(), req.method_string().size()),
        target(req.target().data(), req.target().size()),
        version(req.version()), headers(req.base()) {
    std::cout << target << std::endl;
  }
  std::string_view getRequestType() { return requestType; }
  std::string_view getTarget() { return target; }
  std::string getTargetHost() {
    auto [host, _] =
        parse_url(std::string(target)); // Using structured binding to extract only the host
    return host;
  }
  std::string getTargetPort() {
    auto [_, port] =
        parse_url(std::string(target)); // Using structured binding to extract only the port
    return port;
  }
  int getVersion() { return version; }
  http::fields getHeaders() { return headers; }
  int getID() { return id; }
  http::request<http::string_body> getRequest() { return req; }
};

#endif