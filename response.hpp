#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include <iostream>
#include <string_view>
#include <regex>

namespace http = boost::beast::http;
using namespace boost::asio;
using boost::asio::ip::tcp;

class Response {
protected:
  http::response<http::string_body> res;
  int status_code;
  http::fields headers;
  std::string body;

public:
  Response(http::response<http::string_body> &m_res)
      : res(m_res),
        status_code(res.result_int()),
        headers(res.base()),
        body(res.body()){}

  int getStatusCode() { return status_code; }
  http::fields getHeaders() { return headers; }
  std::string getBody() { return body; }
  http::response<http::string_body> getResponse() { return res; }
};

#endif