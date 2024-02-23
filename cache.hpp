#ifndef CACHE_HPP 
#define CACHE_HPP

#include <string>
#include <map>
#include <boost/beast/http.hpp>
#include <queue>

using namespace std;
using namespace boost::beast::http;
class Cache{
protected:
    int cacheCapacity;
    map<string, response<string_body>> cacheBase;
    queue<string> cacheQueue;
public:
    Cache(int cap):cacheCapacity(cap){}
    void addToCache(string &url, response<string_body> &response);
    void removeFromCache();
    bool isCacheFull();
    bool isInCache(string &url);
    void printToLog();
    response<string_body> *getCachedPage(string &url);
};

#endif