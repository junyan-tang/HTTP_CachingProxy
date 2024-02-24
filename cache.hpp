#ifndef __CACHE_HPP__
#define __CACHE_HPP__
#include "response.hpp"
#include <boost/beast/http.hpp>
#include <map>
#include <queue>
#include <string>
#include <chrono>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace http = boost::beast::http;
class Cache {
protected:
  size_t cacheCapacity;
  std::map<std::string, Response> cacheBase;
  std::queue<std::string> cacheQueue;

public:
  Cache(size_t cap) : cacheCapacity(cap) {}
  void addToCache(std::string &url, Response &response);
  void removeFromCache();
  bool isCacheFull();
  bool isInCache(std::string &url);
  bool checkValidation(std::string &url, int reqID);
  http::response<http::string_body> getCachedPage(std::string &url);
};

#endif