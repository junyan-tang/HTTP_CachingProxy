#include "cache.hpp"
#include "log.hpp"

bool compareExpireTime(http::response<http::string_body> resp, std::string expire_time){
  int max_age = -1;
  if(resp.find("Cache-Control") != resp.end()) {
    std::string cache_control = resp["Cache-Control"].to_string();  
    std::regex max_age_regex(R"(max-age=(\d+))");
    std::smatch match;
    if(std::regex_search(cache_control, match, max_age_regex) && match.size() > 1) {
        max_age = stoi(match[1].str());
    } 
  } 
  
  if(expire_time != ""){
    std::tm tm = {};
    std::istringstream ss(expire_time);
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");

    auto expireTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto now = std::chrono::system_clock::now();
    if(now > expireTime){
      return false;
    }
    else if(max_age == -1){
      return true;
    }
  }
  if(max_age != -1) {
    if(resp.find("Date") != resp.end()){
      std::string date = resp["Date"].to_string();
      std::tm tm = {};
      std::istringstream ss(date);
      ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");
      auto max_age_time = std::chrono::seconds(max_age);
      auto expireTime = std::chrono::system_clock::from_time_t(std::mktime(&tm)) + max_age_time;
      auto now = std::chrono::system_clock::now();
      return (now > expireTime) ? false:true;
    }
  }
  return false;
}
 
bool Cache::checkValidation(http::response<http::string_body> resp){
  if (resp.find(http::field::cache_control) != resp.end()) {
        std::string cache_control = resp.at(http::field::cache_control).to_string();
        if(cache_control.find("no-cache") != std::string::npos){
          return true;
        }
        if(cache_control.find("must-revalidate") != std::string::npos){
          return true;
        }
    }
}

bool Cache::isCacheUsable(std::string_view &uri, int req_id){
  if(!isInCache(uri)){
    logFile << req_id << ": not in cache" << std::endl;
  }
  else{
    Response resp = cacheBase[uri];
    std::string expire_time = resp.checkExpireTime(resp.getResponse());
    if(compareExpireTime(resp.getResponse(), expire_time)){
      logFile << req_id << ": in cache, but expired at " << expire_time << std::endl;
    }
    else if(checkValidation(resp.getResponse())){
      logFile << req_id << ": in cache, requires validation" << std::endl;
    }
    else{
      logFile << req_id << ": in cache, valid" << std::endl;
      return true;
    }
  }
  return false;
}

void Cache::removeFromCache() {
  std::string_view uri = cacheQueue.front();
  cacheBase.erase(uri);
  cacheQueue.pop();
}

void Cache::addToCache(std::string_view &uri, Response &response) {
  if (isCacheFull()) {
    removeFromCache();
  }
  cacheQueue.push(uri);
  cacheBase[uri] = response;
}

bool Cache::isCacheFull() {
  if (cacheQueue.size() >= cacheCapacity) {
    return true;
  }
  return false;
}

bool Cache::isInCache(std::string_view &uri) {
  if (cacheBase.find(uri) == cacheBase.end()) {
    return false;
  }
  return true;
}

http::response<http::string_body> Cache::getCachedPage(std::string_view &uri) {
  Response res = cacheBase[uri];
  return res.getResponse();
}
