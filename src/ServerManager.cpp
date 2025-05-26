#include "../includes/Booter.hpp"
#include "../includes/ResourceValidator.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/ServerManager.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Cgi.hpp"

volatile sig_atomic_t shutdown_requested = 0;
static void handle_signal(const int signal)
{
    if (signal == SIGINT || signal == SIGTERM){
        Logger::info("Signal received, Shutting down server/s gracefully...");
        shutdown_requested = 1;
    }
}

/**
 * Event loop, il programma tramite la funzione bloccante select(), monitora
 * i sockets nelle varie pool (read/write). Se uno del fd cambia stato(I/O) select()
 * esce e la Request viene gestita nel loop interno.
 */
void ServerManager::eventLoop()
{
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);
    while (!shutdown_requested)
    {
        this->initFdSets();

        memset(&this->timeout, 0, sizeof(this->timeout));
        this->timeout.tv_sec = 5;
        this->timeout.tv_usec = 0;

        const time_t currentTime = time(NULL);
        this->handleClientTimeout(currentTime);

        if ((this->fdsChanged = select(this->maxSocket + 1, &this->readPool, &this->writePool, 0, &this->timeout)) < 0){
            Logger::info("'Select()': interrupted");
            continue;
        }
        if (this->fdsChanged == 0)
            continue;

        for (SOCKET fd = 0; fd <= this->maxSocket; ++fd){
            bool clientRegistered = false;
            for (size_t i = 0; i < this->servers.size(); i++){
                if (FD_ISSET(fd, &this->readPool) && this->servers[i].getServerSocket() == fd && !clientRegistered){
                    this->registerNewConnections(fd, &this->servers[i]);
                    clientRegistered = true;
                }
            }
            if (FD_ISSET(fd, &this->readPool) && this->clients.count(fd) > 0){
                this->processRequest(this->clients[fd]);
            }
            else if (FD_ISSET(fd, &this->writePool) && this->clients.count(fd) > 0){
                Cgi& cgi_obj = this->clients[fd]->getCgiObj();
                const int state = cgi_obj.getCGIState();
                if (state == 1 && FD_ISSET(cgi_obj.getPipeIn()[1], &this->writePool))
                    sendCgiBody(fd, clients[fd], cgi_obj);
                else if (state == 1 && FD_ISSET(cgi_obj.getPipeOut()[0], &this->readPool))
                    readCgiBody(fd, clients[fd], cgi_obj);
                else if ((state == 0 || state == 2) && FD_ISSET(fd, &this->writePool))
                    this->sendResponse(fd, this->clients[fd]);
            }
        }
    }
}

void ServerManager::sendCgiBody(const SOCKET fd, const Client *c, Cgi &cgi_obj)
{
    Request *req = c->getRequest();
    Response *resp = c->getResponse();

    const size_t total = req->getBody().length();
    const size_t sent = cgi_obj.getBytesSended();
    if (sent >= total)
    {
        removeFromSet(cgi_obj.getPipeIn()[1], &this->writePool);
        close(cgi_obj.getPipeIn()[1]);
        return;
    }
    Logger::info("Sending body to CGI process " + printFd(fd));
    const ssize_t bytes_sent = write(cgi_obj.getPipeIn()[1], req->getBody().c_str() + sent, total - sent);
    if (bytes_sent < 0)
    {
        Logger::error(__FILE__, "sendCgiBody() Error while sending CGI data to CGI process");
        removeFromSet(cgi_obj.getPipeIn()[1], &this->writePool);
        close(cgi_obj.getPipeIn()[1]);
        resp->setStatusCode(500);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
    }
    else
    {
        cgi_obj.incrementBytesSended(bytes_sent);
        if (cgi_obj.getBytesSended() == total)
        {
            Logger::info("Body fully sent to CGI Process " +  printFd(fd));
            removeFromSet(cgi_obj.getPipeIn()[1], &this->writePool);
            close(cgi_obj.getPipeIn()[1]);
        }
    }
}

void ServerManager::readCgiBody(const SOCKET fd, const Client *c, Cgi &cgi_obj)
{
    Response *resp = c->getResponse();
    
    char buffer[BUFFER_SIZE + 1] = {};
    const long bytes_read = read(cgi_obj.getPipeOut()[0], buffer, BUFFER_SIZE);
    Logger::info("Bytes read from CGI: [" + wb_ltos(bytes_read) + "] " + printFd(fd));
    if (bytes_read < 0){
        Logger::error(__FILE__, "readCgiBody() Error while reading CGI data from CGI process");
        resp->setStatusCode(500);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        removeFromSet(cgi_obj.getPipeOut()[0], &this->readPool);
        close(cgi_obj.getPipeOut()[0]);
        return;
    }
    if (bytes_read == 0){
        removeFromSet(cgi_obj.getPipeOut()[0], &this->readPool);
        close(cgi_obj.getPipeOut()[0]);
        cgi_obj.setCGIState(2);
    } else {
        resp->setBody(resp->getBody() + std::string(buffer));
    }
}

void ServerManager::registerNewConnections(const SOCKET serverFd, Server *s)
{
    Client *c = new Client();

    const SOCKET new_socket = accept(serverFd, reinterpret_cast<sockaddr *>(&c->getAddr()), &c->getAddrLen());
    if (new_socket < 0){
        delete c;
        return;
    }
    if (fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0){
        Logger::error("ServerManager", "Error: 'fcntl()' failed");
        close(new_socket);
        delete c;
        return;
    }

    Logger::info("New connection from " + ServerManager::getClientIP(c) + printFd(new_socket));
    c->setSocketFd(new_socket);
    this->addToSet(new_socket, &this->masterPool);
    c->setServer(s);
    c->updateLastActivity();
    this->clients[new_socket] = c;
}

void ServerManager::processRequest(Client *c)
{
    Request *req = c->getRequest();
    Response *resp = c->getResponse();

    char buffer[BUFFER_SIZE + 1] = {};

    const SOCKET fd = c->getSocketFd();
    const long bytesRecv = recv(fd, buffer, BUFFER_SIZE, 0);
    if (bytesRecv == -1){
        Logger::error("ServerManager", "Error in recv(): " + printFd(fd));
        this->closeClientConnection(fd, c);
        return;
    }

    if (bytesRecv == 0){
        Logger::info("Client closed connection " + printFd(fd));
        this->closeClientConnection(fd, c);
        return;
    }

    if (bytesRecv > 0){
        Logger::info("Received " + wb_ltos(bytesRecv) + " bytes" + printFd(fd));
        const std::string str(buffer, static_cast<std::string::size_type>(bytesRecv));
        req->consume(str);
    }

    if (req->getHasBody() && req->getBodyCounter() > MAX_REQUEST_SIZE){
        Logger::error("ServerManager", "Request body too large " + printFd(fd));
        resp->setStatusCode(413);
        req->setState(StateParsingError);
        return;
    }

    if (req->getState() != StateParsingComplete && req->getState() != StateParsingError)
        return;

    Logger::info("Request was consumed assigning server" + printFd(fd));
    this->assignServer(c);

    if (req->getState() == StateParsingComplete) {
        Logger::info("Request parsing complete, validating resource" + printFd(fd));

        ResourceValidator::validateResource(c, c->getServer());

        if (resp->getStatus() != 200) {
            this->removeFromSet(fd, &this->readPool);
            this->addToSet(fd, &this->writePool);
            return;
        }

        if (c->getCgiObj().getCGIState() == 1) {
            Logger::info("Request is a CGI request " + printFd(fd));
            this->addToSet(c->getCgiObj().getPipeIn()[1], &this->writePool);
            this->addToSet(c->getCgiObj().getPipeOut()[0], &this->readPool);
        }
    }

    if (req->getState() == StateParsingError) {
        Logger::error("ServerManager", "Error parsing reqrequest" + printFd(fd));
        resp->setStatusCode(req->getError());
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
    }

    this->removeFromSet(fd, &this->readPool);
    this->addToSet(fd, &this->writePool);

    c->updateLastActivity();
}

void ServerManager::sendResponse(const SOCKET fd, Client *c)
{
    Request *req = c->getRequest();
    Response *resp = c->getResponse();

    c->updateLastActivity();

    resp->prepareResponse(c, req);

    const long bytes_sent = send(fd, resp->getResponse().c_str(), resp->getResponse().size(), 0);
    if (bytes_sent == -1){
        Logger::error("ServerManager", "Send failed: " + printFd(fd));
        this->closeClientConnection(fd, c);
        return;
    }
    Logger::info("Response sent successfully (" + wb_ltos(bytes_sent) + " bytes) " + printFd(fd));
    if (req->getHeaders()["connection"] == "close"){
        Logger::info("Closing connection as requested by client" + printFd(fd));
        this->closeClientConnection(fd, c);
    } else {
        c->reset();
        this->closeClientConnection(fd, c);
        Logger::info("Connection kept alive for next request" + printFd(fd));
    }
}
