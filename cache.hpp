#include <string>
#include <map>
#include <queue>

using namespace std;
class Cache{
protected:
    //total cache responses
    int cacheCapacity;
    //Response haven't be designed.
    map<string, Response> cacheBase;
    //first in first out(store cache)
    queue<string> cacheQueue;

};