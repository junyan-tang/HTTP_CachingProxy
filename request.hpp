#ifndef REQUEST_HPP 
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

using namespace boost::beast::http;
using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

class Request{
protected:
    string requestType;
    request<string_body> req;
public:
    Request(boost::beast::flat_buffer &buffer, tcp::socket &socket){
        read(socket, buffer, req);
    }
    void parseRequest();
    boost::beast::string_view test(){
        return req.method_string();
    }
};

#endif