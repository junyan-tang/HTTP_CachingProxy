#ifndef __CACHE_HPP__
#define __CACHE_HPP__
#include "response.hpp"
#include <boost/beast/http.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <queue>
#include <sstream>
#include <string>

namespace http = boost::beast::http;
class Cache {
protected:
  size_t cacheCapacity;
  std::map<std::string_view, Response> cacheBase;
  std::queue<std::string_view> cacheQueue;

public:
  Cache(size_t cap) : cacheCapacity(cap) {}
  void addToCache(std::string_view &uri, Response &response);
  void removeFromCache();
  bool isCacheFull();
  bool isInCache(std::string_view &uri);
  bool isCacheUsable(std::string_view &uri, int reqID);
  bool checkValidation(http::response<http::string_body> res);
  http::response<http::string_body> getCachedPage(std::string_view &uri);
};

#endif