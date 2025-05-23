#include "../../includes/ServerManager.hpp"
#include "../../includes/Booter.hpp"
#include "../../includes/Parser.hpp"
#include "../../includes/Request.hpp"
#include "../../includes/Response.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Utils.hpp"
#include "../../includes/Logger.hpp"

Client *ServerManager::getClient(SOCKET clientFd){
    
    if (this->_clients_map.count(clientFd) > 0){
        return this->_clients_map[clientFd];
    }
    return new Client(clientFd);
}

void ServerManager::removeClient(SOCKET fd){
    if (this->_clients_map.count(fd) > 0) {
        delete this->_clients_map[fd];
        this->_clients_map.erase(fd);
    }
    if(FD_ISSET(fd, &this->_masterPool)){
        removeFromSet(fd, &this->_masterPool);
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    fd = -1;
}

void ServerManager::addServer(Server &server){
    this->_servers_map.push_back(server);
}

void ServerManager::addToSet(SOCKET fd, fd_set *fdSet){
    FD_SET(fd, fdSet);
    this->_maxSocket = std::max(this->_maxSocket, fd);
}


void ServerManager::removeFromSet(SOCKET fd, fd_set *targetSet) {
    FD_CLR(fd, targetSet);

    if (fd == this->_maxSocket) {
        SOCKET newMax = 0;
        for (SOCKET i = this->_maxSocket - 1; i >= 0; --i) {
            if (FD_ISSET(i, &this->_readPool) || FD_ISSET(i, &this->_writePool)) {
                newMax = i;
                break;
            }
        }
        this->_maxSocket = newMax;
    }
}


const std::string ServerManager::getClientIP(Client *client){
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo((sockaddr*)&client->getAddr(), client->getAddrLen(), address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return std::string(address_info);
}

void ServerManager::closeClientConnection(SOCKET fd, Client* client) 
{
    if (client == NULL) {
        if (fd != -1) {
            removeFromSet(fd, &this->_masterPool);
            removeFromSet(fd, &this->_readPool);
            removeFromSet(fd, &this->_writePool);
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
        return;
    }

    removeFromSet(fd, &this->_masterPool);
    removeFromSet(fd, &this->_readPool);
    removeFromSet(fd, &this->_writePool);
    
    int socket_status;
    socklen_t len = sizeof(socket_status);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &socket_status, &len) == 0) {
        if (shutdown(fd, SHUT_RDWR) < 0 && errno != ENOTCONN) {
            Logger::error("ServerManager", "Error shutting down socket: " + std::string(strerror(errno)));
        }
        if (close(fd) < 0) {
            Logger::error("ServerManager", "Error closing socket: " + std::string(strerror(errno)));
        }
    } else {
        Logger::info("Socket already closed [" + intToStr(fd) + "]");
    }
    this->_clients_map.erase(fd);
    delete client;
    
}


void ServerManager::handleClientTimeout(time_t currentTime) {
    std::vector<SOCKET> clientsToRemove;
    
    for (std::map<SOCKET, Client*>::iterator it = this->_clients_map.begin(); it != this->_clients_map.end(); ++it) {
        Client* client = it->second;
        if (currentTime - client->getLastActivity() > TIMEOUT_SEC) {
            clientsToRemove.push_back(it->first);
        }
    }
    
    for (std::vector<SOCKET>::iterator it = clientsToRemove.begin(); it != clientsToRemove.end(); ++it) {
        SOCKET fd = *it;
        Client* client = this->_clients_map[fd];
        Logger::info("Connection timeout, closing connection [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
    }
}

void ServerManager::initFdSets()
{
    for (size_t i = 0; i < this->_servers_map.size(); i++){
        SOCKET serverSocket = this->_servers_map[i].getServerSocket();
        
        this->addToSet(serverSocket, &this->_masterPool);
        this->addToSet(serverSocket, &this->_readPool);
        
        if (serverSocket > this->_maxSocket){
            this->_maxSocket = serverSocket;
        }
    }
    std::map<SOCKET, Client *>::iterator it;
    for (it = this->_clients_map.begin(); it != this->_clients_map.end(); ++it){
        SOCKET clientSocket = it->first;
        Client *client = it->second;

        this->addToSet(clientSocket, &this->_masterPool);
        
        Request *req = client->getRequest();
        if (req && (req->getState() == StateParsingComplete || req->getState() == StateParsingError)){
            this->addToSet(clientSocket, &this->_writePool);
        } else {
            this->addToSet(clientSocket, &this->_readPool);
        }
        
        if (clientSocket > this->_maxSocket){
            this->_maxSocket = clientSocket;
        }

        Cgi& cgi = client->getCgiObj();
            if (cgi.getCGIState() == 1){
                int cgiIn = cgi.getPipeIn()[1];
                int cgiOut = cgi.getPipeOut()[0];
                
                if (cgiIn >= 0 && fcntl(cgiIn, F_GETFD) != -1)
                this->addToSet(cgiIn, &this->_writePool);
                if (cgiOut >= 0 && fcntl(cgiOut, F_GETFD) != -1)
                this->addToSet(cgiOut, &this->_readPool);
                
                if (cgiIn > this->_maxSocket)
                this->_maxSocket = cgiIn;
                if (cgiOut > this->_maxSocket)
                this->_maxSocket = cgiOut;
        }
    }
}

void ServerManager::assignServer(Client *client){

    Request *request = client->getRequest();

    if (request->getHeaders().find("host") != request->getHeaders().end()){

        const std::string reqPort = request->getHeaders()["host"].find_last_of(":") != std::string::npos
            ? request->getHeaders()["host"].substr(request->getHeaders()["host"].find_last_of(":") + 1, request->getHeaders()["host"].size())
            : "80";

        const std::string reqHost = request->getHeaders()["host"].find_last_of(":") != std::string::npos
            ? request->getHeaders()["host"].substr(0, request->getHeaders()["host"].find_last_of(":"))
            : request->getHeaders()["host"];

        const std::string reqHostPort = reqHost + ":" + reqPort;
        for (size_t i = 0; i < this->_servers_map.size(); i++){
            std::map<std::string, std::vector<std::string> > serverDir = this->_servers_map[i].getServerDir();
            if (serverDir.find("server_name") != serverDir.end() && serverDir["server_name"][0] == request->getHeaders()["host"] && this->_servers_map[i].getHostPortKey() == reqHostPort){
                client->setServer(&this->_servers_map[i]);
                break;
            }
        }
    }
}

std::vector<Server> ServerManager::getServerMap(void) { return this->_servers_map; }

ServerManager::ServerManager()
{
    this->_maxSocket = 0;
    FD_ZERO(&this->_readPool);
    FD_ZERO(&this->_writePool);
    FD_ZERO(&this->_masterPool);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
}

ServerManager::~ServerManager()
{
    Logger::info("ServerManager Destructor() Called!!!");
}
