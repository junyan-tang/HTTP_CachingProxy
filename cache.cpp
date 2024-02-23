#include "cache.hpp"
#include "log.hpp"

void Cache::printToLog() {}

void Cache::removeFromCache() {
  std::string url = cacheQueue.front();
  cacheBase.erase(url);
  cacheQueue.pop();
}

void Cache::addToCache(std::string &url,
                       http::response<http::string_body> &response) {
  if (!isInCache(url)) {
    if (isCacheFull()) {
      removeFromCache();
    }
    cacheBase[url] = response;
    cacheQueue.push(url);
  } else {
    // handle cache
  }
  cacheBase[url] = response;
}

bool Cache::isCacheFull() {
  if (cacheQueue.size() >= cacheCapacity) {
    return true;
  }
  return false;
}

bool Cache::isInCache(std::string &url) {
  if (cacheBase.find(url) == cacheBase.end()) {
    return false;
  }
  return true;
}

http::response<http::string_body> Cache::getCachedPage(std::string &url) {
  http::response<http::string_body> res = cacheBase[url];
  return res;
}
