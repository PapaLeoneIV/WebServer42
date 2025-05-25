#ifndef BOOTER_HPP
#define BOOTER_HPP

#include "Server.hpp"
#include "ServerManager.hpp"

typedef int SOCKET;

class Booter {
    public:
    static void    boot(std::vector<Server> &servers,  ServerManager &sm);

};


#endif