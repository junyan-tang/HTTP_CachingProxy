#ifndef LOG_HPP
#define LOG_HPP

#include <fstream>
#include <iostream>

//path: /var/log/erss/proxy.log
inline std::ofstream logFile("./log.txt", std::ios::app);

#endif