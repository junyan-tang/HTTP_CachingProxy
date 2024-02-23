#ifndef REQUEST_HPP
#define REQUEST_HPP

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
  string_view requestType;

public:
  Request(http::request<http::string_body> &m_req)
      : req(m_req),
        requestType(req.method_string().data(), req.method_string().size()) {}
};

#endif