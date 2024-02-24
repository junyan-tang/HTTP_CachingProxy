#include <fstream>
#include <iostream>

//path: /var/log/erss/proxy.log
extern std::ofstream logFile("./log.txt", std::ios::app);
