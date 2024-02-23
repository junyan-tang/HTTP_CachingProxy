#ifndef __CACHE_HPP__
#define __CACHE_HPP__

#include <boost/beast/http.hpp>
#include <map>
#include <queue>
#include <string>

namespace http = boost::beast::http;
class Cache {
protected:
  size_t cacheCapacity;
  std::map<std::string, http::response<http::string_body>> cacheBase;
  std::queue<std::string> cacheQueue;

public:
  Cache(size_t cap) : cacheCapacity(cap) {}
  void addToCache(std::string &url, http::response<http::string_body> &response);
  void removeFromCache();
  bool isCacheFull();
  bool isInCache(std::string &url);
  void printToLog();
  http::response<http::string_body> getCachedPage(std::string &url);
};

#endif