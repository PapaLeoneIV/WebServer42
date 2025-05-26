#include "../../includes/ServerManager.hpp"
#include "../../includes/ResourceValidator.hpp"
#include "../../includes/Request.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Utils.hpp"
#include "../../includes/Logger.hpp"

std::vector<Server> ServerManager::getServerMap() { return this->servers; }

void ServerManager::addServer(const Server &s){this->servers.push_back(s);}

void ServerManager::addToSet(const SOCKET fd, fd_set *fdSet){ FD_SET(fd, fdSet); this->maxSocket = std::max(this->maxSocket, fd);}


void ServerManager::removeFromSet(SOCKET fd, fd_set *targetSet) {
    FD_CLR(fd, targetSet);

    if (fd == this->maxSocket) {
        SOCKET newMax = 0;
        for (SOCKET i = this->maxSocket - 1; i >= 0; --i) {
            if (FD_ISSET(i, &this->readPool) || FD_ISSET(i, &this->writePool)) {
                newMax = i;
                break;
            }
        }
        this->maxSocket = newMax;
    }
}

std::string ServerManager::getClientIP(Client *c){
    static char address_info[INET6_ADDRSTRLEN];
    getnameinfo(reinterpret_cast<sockaddr *>(&c->getAddr()), c->getAddrLen(), address_info, sizeof(address_info), 0, 0, NI_NUMERICHOST);
    return std::string(address_info);
}

void ServerManager::closeClientConnection(const SOCKET fd, const Client* c)
{
    if (c == NULL) {
        if (fd != -1) {
            removeFromSet(fd, &this->masterPool);
            removeFromSet(fd, &this->readPool);
            removeFromSet(fd, &this->writePool);
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
        return;
    }

    removeFromSet(fd, &this->masterPool);
    removeFromSet(fd, &this->readPool);
    removeFromSet(fd, &this->writePool);
    
    int socket_status;
    socklen_t len = sizeof(socket_status);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &socket_status, &len) == 0) {
        if (shutdown(fd, SHUT_RDWR) < 0) {
            Logger::error("ServerManager", "Error shutting down socket");
        }
        if (close(fd) < 0) {
            Logger::error("ServerManager", "Error closing socket");
        }
    } else {
        Logger::info("Socket already closed [" + wb_itos(fd) + "]");
    }
    this->clients.erase(fd);
    delete c;
    
}

void ServerManager::handleClientTimeout(const time_t currentTime) {
    std::vector<SOCKET> clientsToRemove;
    
    for (std::map<SOCKET, Client*>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
        Client* c = it->second;
        if (currentTime - c->getLastActivity() > TIMEOUT_SEC) {
            clientsToRemove.push_back(it->first);
        }
    }
    
    for (std::vector<SOCKET>::iterator it = clientsToRemove.begin(); it != clientsToRemove.end(); ++it) {
        SOCKET fd = *it;
        Client* c = this->clients[fd];
        Logger::info("Connection timeout, closing connection [" + wb_itos(fd) + "]");
        this->closeClientConnection(fd, c);
    }
}

void ServerManager::initFdSets()
{
    FD_ZERO(&this->readPool);
    FD_ZERO(&this->writePool);
    
    for (size_t i = 0; i < this->servers.size(); i++){
        const SOCKET serverSocket = this->servers[i].getServerSocket();
        
        this->addToSet(serverSocket, &this->masterPool);
        this->addToSet(serverSocket, &this->readPool);
        
        if (serverSocket > this->maxSocket){
            this->maxSocket = serverSocket;
        }
    }
    for (std::map<SOCKET, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); ++it){
        SOCKET clientSocket = it->first;
        Client *c = it->second;

        this->addToSet(clientSocket, &this->masterPool);
        
        Request *req = c->getRequest();
        if (req && (req->getState() == StateParsingComplete || req->getState() == StateParsingError)){
            this->addToSet(clientSocket, &this->writePool);}
        else { 
            this->addToSet(clientSocket, &this->readPool);
        }
        
        if (clientSocket > this->maxSocket){ this->maxSocket = clientSocket;}

        Cgi& cgi = c->getCgiObj();
        if (cgi.getCGIState() == 1){
            int cgiIn = cgi.getPipeIn()[1];
            int cgiOut = cgi.getPipeOut()[0];
            
            if (cgiIn >= 0 && fcntl(cgiIn, F_GETFD) != -1)
                this->addToSet(cgiIn, &this->writePool);
            if (cgiOut >= 0 && fcntl(cgiOut, F_GETFD) != -1)
                this->addToSet(cgiOut, &this->readPool);
            
            if (cgiIn > this->maxSocket)
                this->maxSocket = cgiIn;
            if (cgiOut > this->maxSocket)
                this->maxSocket = cgiOut;
        }
    }
}

void ServerManager::assignServer(Client *c){

    Request *req = c->getRequest();

    if (req->getHeaders().find("host") != req->getHeaders().end()){

        const std::string reqPort = req->getHeaders()["host"].find_last_of(':') != std::string::npos
            ? req->getHeaders()["host"].substr(req->getHeaders()["host"].find_last_of(':') + 1, req->getHeaders()["host"].size())
            : "80";

        const std::string reqHost = req->getHeaders()["host"].find_last_of(':') != std::string::npos
            ? req->getHeaders()["host"].substr(0, req->getHeaders()["host"].find_last_of(':'))
            : req->getHeaders()["host"];

        const std::string reqHostPort = reqHost + ":" + reqPort;
        for (size_t i = 0; i < this->servers.size(); i++){
            std::map<std::string, std::vector<std::string> > serverDir = this->servers[i].getServerDir();
            if (serverDir.find("server_name") != serverDir.end() &&
                serverDir["server_name"][0] == reqHost &&
                serverDir.find("listen") != serverDir.end() &&
                serverDir["listen"][0] == reqPort){
                    c->setServer(&this->servers[i]);
                    break;
                }
        }
    }
}


ServerManager::ServerManager() : timeout(), masterPool(), readPool(), writePool(){
    this->maxSocket = 0;
    FD_ZERO(&this->readPool);
    FD_ZERO(&this->writePool);
    FD_ZERO(&this->masterPool);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    fdsChanged = 0;
}

ServerManager::~ServerManager(){}
