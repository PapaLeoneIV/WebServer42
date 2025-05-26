#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <string>
#include <map>
#include <csignal>
#include "Utils.hpp"
#include "Server.hpp"
#include "Client.hpp"

typedef int SOCKET;

class ServerManager {
public:
    ServerManager();
    ~ServerManager();

    // Core loop
    void eventLoop();

    // Client and req handling
    void registerNewConnections(SOCKET serverFd, Server *s);
    void processRequest(Client *c);
    void sendResponse(SOCKET fd, Client *c);
    void sendCgiBody(SOCKET fd, const Client *c, Cgi &cgi_obj);
    void readCgiBody(SOCKET fd, const Client *c, Cgi &cgi_obj);
    void assignServer(Client *c);
    void closeClientConnection(SOCKET fd, const Client *c);
    void handleClientTimeout(time_t currentTime);

    // FD set management
    void initFdSets();
    void addToSet(SOCKET fd, fd_set *fdSet);
    void removeFromSet(SOCKET fd, fd_set *fdSet);

    // Server and c registry
    void addServer(const Server &s);

    static std::string getClientIP(Client *c);
    std::vector<Server> getServerMap();

private:
    struct timeval timeout;

    fd_set masterPool;
    fd_set readPool;
    fd_set writePool;

    int maxSocket;

    std::map<SOCKET, Client*> clients;
    std::vector<Server> servers;

	int fdsChanged;
};

#endif // SERVERMANAGER_HPP
