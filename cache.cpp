#include "cache.hpp"
#include "log.hpp"

// bool compareExpireTime(http::response<http::string_body> resp){
//   if(expire_time != ""){
//     std::tm tm = {};
//     std::istringstream ss(expire_time);
//     ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");

//     auto expireTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
//     auto now = std::chrono::system_clock::now();
//     if(now > expireTime){
//       return false;
//     }
//     else if(max_age_str == ""){
//       return true;
//     }
//   }
//   if(max_age_str != ""){
//     int max_age = std::stoi(max_age_str);
//     auto expireTime = resp_time + max_age;
//     auto now = std::chrono::system_clock::now();
//     return (now > expireTime) ? false:true;
//   }
//   return false;
// }
  // std::string checkExpireTime(){
  //   std::string expire_time;
  //   auto it = res.find(http::field::expires);
  //   if(it != res.end()){
  //     expire_time = it->value().to_string();
  //   }
  //   else{
  //     expire_time = "";
  //   }
  //   return expire_time;
  // }
  // std::string getMaxAge(){
  //   if(res.find("Cache-Control") != res.end()) {
  //       std::string cache_control = res["Cache-Control"].to_string();  
  //       std::regex max_age_regex(R"(max-age=(\d+))");
  //       std::smatch match;
  //       if(std::regex_search(cache_control, match, max_age_regex) && match.size() > 1) {
  //           return match[1].str();
  //       } 
  //   } 
  //   return "";
  // }
  // std::string getResponseTime(){
  //   return ;
  // }

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
    //if(compareExpireTime(resp.getResponse()){
    if(false){
      //logFile << req_id << ": in cache, but expired at " << resp.checkExpireTime() << std::endl;
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
