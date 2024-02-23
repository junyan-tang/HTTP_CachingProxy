#ifndef CACHE_HPP 
#define CACHE_HPP

#include <string>
#include <map>
#include <boost/beast/http.hpp>
#include <queue>

using namespace std;
namespace http = boost::beast::http;
class Cache{
protected:
    int cacheCapacity;
    map<string, http::response<http::string_body>> cacheBase;
    queue<string> cacheQueue;
public:
    Cache(int cap):cacheCapacity(cap){}
    void addToCache(string &url, http::response<http::string_body> &response);
    void removeFromCache();
    bool isCacheFull();
    bool isInCache(string &url);
    void printToLog();
    http::response<http::string_body> &getCachedPage(string &url);
};

#endif