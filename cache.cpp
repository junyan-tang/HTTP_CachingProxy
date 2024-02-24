#include "cache.hpp"
#include "log.hpp"

bool compareExpireTime(std::string expire_time){
    if(expire_time == ""){
      return false;
    }
    std::tm tm = {};
    std::istringstream ss(expire_time);
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");

    auto expireTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto now = std::chrono::system_clock::now();
    return (now > expireTime) ? true: false;
}

bool Cache::checkValidation(std::string &url, int req_id){
  if(!isInCache(url)){
    logFile << req_id << ": not in cache" << std::endl;
    return false;
  }
  else{
    Response resp = cacheBase[url];
    if(compareExpireTime(resp.checkExpireTime())){
      logFile << req_id << ": in cache, but expired at " << resp.checkExpireTime() << std::endl;
      return false;
    }
    //check validation place holder
    else if(false){
      logFile << req_id << ": in cache, requires validation" << std::endl;
      return false;
    }
    else{
      logFile << req_id << ": in cache, valid" << std::endl;
      return true;
    }
  }
  
}

void Cache::removeFromCache() {
  std::string url = cacheQueue.front();
  cacheBase.erase(url);
  cacheQueue.pop();
}

void Cache::addToCache(std::string &url,
                       Response &response) {
  if (isCacheFull()) {
    removeFromCache();
  }
  cacheQueue.push(url);
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
  Response res = cacheBase[url];
  return res.getResponse();
}
