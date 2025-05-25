#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP
#include <csignal>
#include <string>
#include <map>
#include "../includes/Utils.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

typedef int SOCKET;

class ServerManager {
public:
    ServerManager();
    ~ServerManager();

    // Core loop
    void eventLoop();

    // Client and req handling
    void registerNewConnections(SOCKET serverFd, Server *server);
    void processRequest(Client *c);
    void sendResponse(SOCKET fd, Client *c);
    void sendCgiBody(SOCKET fd, Client *c, Cgi &cgi_obj);
    void readCgiBody(SOCKET fd, Client *c, Cgi &cgi_obj);
    void assignServer(Client *c);
    void closeClientConnection(SOCKET fd, Client *c);
    void handleClientTimeout(time_t currentTime);

    // FD set management
    void initFdSets();
    void addToSet(SOCKET fd, fd_set *fdSet);
    void removeFromSet(SOCKET fd, fd_set *fdSet);

    // Server and c registry
    void addServer(Server &server);
    void removeClient(SOCKET fd);
    const std::string getClientIP(Client *c);
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
