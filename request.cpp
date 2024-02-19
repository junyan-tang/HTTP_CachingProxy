#include <boost/beast.hpp>
#include <boost/asio.hpp>

using namespace boost::beast;
using namespace boost::asio;

void parseRequest(){
    http::request<http::string_body> request;
    //http::read(socket, buffer, request);
}