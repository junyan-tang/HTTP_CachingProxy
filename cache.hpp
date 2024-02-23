#ifndef __CACHE_HPP__
#define __CACHE_HPP__

#include <boost/beast/http.hpp>
#include <map>
#include <queue>
#include <string>

using namespace std;
namespace http = boost::beast::http;
class Cache {
protected:
  size_t cacheCapacity;
  map<string, http::response<http::string_body>> cacheBase;
  queue<string> cacheQueue;

public:
  Cache(size_t cap) : cacheCapacity(cap) {}
  void addToCache(string &url, http::response<http::string_body> &response);
  void removeFromCache();
  bool isCacheFull();
  bool isInCache(string &url);
  void printToLog();
  http::response<http::string_body> getCachedPage(string &url);
};

#endif