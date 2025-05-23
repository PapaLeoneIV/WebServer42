#ifndef BOOTER_HPP
#define BOOTER_HPP

#include "Server.hpp"

typedef int SOCKET;

class Booter {
    public:
    static void    bootServer    (Server *server,  std::string host,  std::string port);
};


#endif