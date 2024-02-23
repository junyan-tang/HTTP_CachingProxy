#include "cache.hpp"
#include "log.hpp"

void Cache::printToLog(){

}

void Cache::removeFromCache(){

}

void Cache::addToCache(string &url, response<string_body> &response){
    if(!isInCache(url)){
        if(isCacheFull()){

        }
        cacheQueue.push(url);
    }
    else{
        //handle cache
    }
    cacheBase[url] = response;
}

bool Cache::isCacheFull(){
    if(cacheQueue.size() >= cacheCapacity){
        return true;
    }
    return false;
}

bool Cache::isInCache(string &url){
    if(cacheBase.find(url) == cacheBase.end()){
        return false;
    }
    return true;
}

response<string_body> * Cache::getCachedPage(string &url){
    response<string_body> * res = &cacheBase[url];
    return res;
}
