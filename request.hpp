#ifndef REQUEST_HPP 
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

namespace http = boost::beast::http;
using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

class Request{
protected:
    string_view requestType;
    http::request<http::string_body> req;
public:
    Request(http::request<http::string_body> &m_req):req(m_req), 
    requestType(req.method_string().data(), req.method_string().size()){}   
};

#endif