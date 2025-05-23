#include "../includes/Booter.hpp"
#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"

void Booter::bootServer(Server *server, std::string host, std::string port){
    struct addrinfo *addrinfo = NULL;

    if (getaddrinfo(host.c_str(), port.c_str(), &server->getHints(), &addrinfo)) {
        Logger::error(__FILE__, ErrToStr(ERR_RESOLVE_ADDR));
        exit(ERR_RESOLVE_ADDR);
    }

    server->setBindAddress(addrinfo);

    SOCKET socket_fd;
    int optval = 1;
    
    socket_fd = socket((server->getHints()).ai_family, (server->getHints()).ai_socktype, (server->getHints()).ai_protocol);

    if (socket_fd == -1){
        Logger::error(__FILE__, ErrToStr(ERR_SOCK_CREATION));
        exit(ERR_SOCK_CREATION);
    }

    server->setServerSocket(socket_fd);

    if (setsockopt(server->getServerSocket(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
        Logger::error(__FILE__, ErrToStr(ERR_SOCKET_NBLOCK));
        exit(ERR_SOCKET_NBLOCK);
    }

    if(fcntl(server->getServerSocket(), F_SETFL, O_NONBLOCK) < 0){
        Logger::error(__FILE__, ErrToStr(ERR_FCNTL));
        exit(ERR_FCNTL);
    }

    if(bind(server->getServerSocket(), server->getBindAddrss()->ai_addr, server->getBindAddrss()->ai_addrlen) == -1){
        Logger::error(__FILE__, ErrToStr(ERR_BIND));
        exit(ERR_BIND);
    }

    if(listen(server->getServerSocket(), 10) == -1){
        Logger::error(__FILE__, ErrToStr(ERR_LISTEN));
        exit(ERR_LISTEN);
    }

    freeaddrinfo(server->getBindAddrss());

}
