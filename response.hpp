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
  int version;
  http::fields headers;
  std::string body;

public:
  Response(http::response<http::dynamic_body> &m_res)
      : res(m_res),
        status_code(res.result_int()),
        headers(res.base()),
        body(res.body()),
        version(res.version()){}
  Response(){}

  int getStatusCode() { return status_code; }
  http::fields getHeaders() { return headers; }
  std::string getBody() { return body; }
  http::response<http::string_body> getResponse() { return res; }
  std::string checkExpireTime(){
    std::string expire_time;
    auto it = res.find(http::field::expires);
    if(it != res.end()){
      expire_time = it->value().to_string();
    }
    else{
      expire_time = "";
    }
    return expire_time;
  }

  std::string getFirstLine(){
    std::ostringstream ss;
    ss << "HTTP/" << version / 10 << "." << version % 10 << " " << status_code << " " << res.reason();
    return ss.str();
  }

  std::string isCacheable(){
    if(status_code != 200){
      std::ostringstream ss;
      ss << res.reason();
      return ss.str();
    }
    if(res.find("Cache-Control") != res.end()) {
      std::string cache_control = res["Cache-Control"].to_string();  
      if(cache_control.find("no-store") != std::string::npos){
        return "This page can't be cached.";
      }
      if(cache_control.find("private") != std::string::npos ){
        return "This page is private.";
      }



       if(res.find("ETag") != res.end()) {
            std::cout << "ETag: " << res["ETag"] << std::endl;
        } else {
            std::cout << "ETag header not found in the response." << std::endl;
        }
    } 
    
    return "";
  } 
};

#endif