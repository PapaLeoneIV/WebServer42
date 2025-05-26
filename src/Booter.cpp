#include "../includes/Booter.hpp"
#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"

void Booter::boot(std::vector<Server> &servers, ServerManager &sm)
{
    std::vector<std::string> hostPortPairs;
    for (size_t i = 0; i < servers.size(); i++)
    {
        Server &s = servers[i];
        std::string host = s.getServerDir()["host"][0];
        std::string port = s.getServerDir()["listen"][0];
        s.sethostPortPair(host + ":" + port);
        bool shouldBoot = true;
        for (size_t j = 0; j < hostPortPairs.size(); j++)
        {
            if (s.gethostPortPair() == hostPortPairs[j]){
                s.setServerSocket(servers[j].getServerSocket());
                shouldBoot = false;
            }
        }
        hostPortPairs.push_back(s.gethostPortPair());
        if (shouldBoot)
        {
            struct addrinfo *addrinfo = NULL;

            if (getaddrinfo(host.c_str(), port.c_str(), &s.getHints(), &addrinfo)){
                Logger::error(__FILE__, ErrToStr(ERR_RESOLVE_ADDR));
                exit(ERR_RESOLVE_ADDR);
            }

            s.setBindAddress(addrinfo);

            int optval = 1;

            SOCKET socket_fd = socket((s.getHints()).ai_family, (s.getHints()).ai_socktype, (s.getHints()).ai_protocol);

            if (socket_fd == -1){
                Logger::error(__FILE__, ErrToStr(ERR_SOCK_CREATION));
                exit(ERR_SOCK_CREATION);
            }

            s.setServerSocket(socket_fd);

            if (setsockopt(s.getServerSocket(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
                Logger::error(__FILE__, ErrToStr(ERR_SOCKET_NBLOCK));
                exit(ERR_SOCKET_NBLOCK);
            }

            if (fcntl(s.getServerSocket(), F_SETFL, O_NONBLOCK) < 0){
                Logger::error(__FILE__, ErrToStr(ERR_FCNTL));
                exit(ERR_FCNTL);
            }

            if (bind(s.getServerSocket(), s.getBindAddress()->ai_addr, s.getBindAddress()->ai_addrlen) == -1){
                Logger::error(__FILE__, ErrToStr(ERR_BIND));
                exit(ERR_BIND);
            }

            if (listen(s.getServerSocket(), 10) == -1){
                Logger::error(__FILE__, ErrToStr(ERR_LISTEN));
                exit(ERR_LISTEN);
            }

            freeaddrinfo(s.getBindAddress());
        }
        Logger::info("Server started on " + host + ":" += port);
        sm.addServer(s);
    }
}