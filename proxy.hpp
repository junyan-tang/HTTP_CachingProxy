#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

class Proxy{
protected:
    const char *hostname;
    const char *port;
public:
    Proxy():hostname(NULL), port("12345"){}
    Proxy(char *hostname):hostname(hostname), port(NULL){}

    void processGET();
    void processCONNECT();
    void processPOST();
    void cacheResponse();
    int createServerSocket();
    int accpetClientConnection(int socket_fd);
    int createClientSocket(char *hostname);
};