#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include <iostream>
#include <regex>
#include <string_view>

namespace http = boost::beast::http;
using namespace boost::asio;
using boost::asio::ip::tcp;

inline std::string getETag(http::response<http::dynamic_body> res){
    auto etag = res.find(http::field::etag);
    if (etag == res.end()){
      return "";
    } else {
      return etag->value().to_string();
    }
  }

inline std::string getLastModified(http::response<http::dynamic_body> res){
  auto last_modified = res.find(http::field::last_modified);
  if (last_modified == res.end()){
    return "";
  } else {
    return last_modified->value().to_string();
  }
}

class Response {
protected:
  http::response<http::dynamic_body> res;
  int status_code;
  int version;
  http::fields headers;

public:
  Response(http::response<http::dynamic_body> &m_res)
      : res(m_res), status_code(res.result_int()), version(res.version()),
        headers(res.base()) {}
  Response() {}

  int getStatusCode() { return status_code; }
  http::fields getHeaders() { return headers; }
  http::response<http::dynamic_body> getResponse() { return res; }
  std::string checkExpireTime() {
    std::string expire_time;
    auto it = res.find(http::field::expires);
    if (it != res.end()) {
      expire_time = it->value().to_string();
    } else {
      expire_time = "";
    }
    return expire_time;
  }

  std::string getFirstLine() {
    std::ostringstream ss;
    ss << "HTTP/" << version / 10 << "." << version % 10 << " " << status_code
       << " " << res.reason();
    return ss.str();
  }

  std::string isCacheable() {
    if (status_code != 200) {
      std::ostringstream ss;
      ss << res.reason();
      return ss.str();
    }
    if (res.find("Cache-Control") != res.end()) {
      std::string cache_control = res["Cache-Control"].to_string();
      if (cache_control.find("no-store") != std::string::npos) {
        return "This page can't be cached.";
      }
      if(cache_control.find("private") != std::string::npos ){
        return "This page is private.";
      }
    }
    if(getETag(res) == "" && getLastModified(res) == ""){
      return "This page has no ETag and Last-Modified field.";
    }
    return "";
  }

  
};

#endif