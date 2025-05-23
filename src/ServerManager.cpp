#include "../includes/Booter.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/ServerManager.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Cgi.hpp"

/**
 * Event loop, il programma tramite la funzione bloccante select(), monitora
 * i sockets nelle varie pool (read/write). Se uno degl fd switcha stato(I/O) select()
 * esce e la Request viene gestita nel loop interno.
 */
volatile sig_atomic_t shutdown_requested = 0;
static void handle_signal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        Logger::info("Signal received, Shutting down server gracefully...");
        shutdown_requested = 1;
    }
}

void ServerManager::eventLoop()
{
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);
    while (!shutdown_requested)
    {
        int fds_changed = 0;

        FD_ZERO(&this->_readPool);
        FD_ZERO(&this->_writePool);

        this->initFdSets();

        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        time_t currentTime = time(NULL);
        this->handleClientTimeout(currentTime);

        if ((fds_changed = select(this->_maxSocket + 1, &this->_readPool, &this->_writePool, 0, &timeout)) < 0)
        {
            Logger::info("Select: interrupted");
            continue;
        }
        if (fds_changed == 0)
            continue;

        for (SOCKET fd = 0; fd <= this->_maxSocket; ++fd)
        {

            bool clientRegistred = false;
            for (size_t i = 0; i < this->_servers_map.size(); i++)
            {
                if (FD_ISSET(fd, &this->_readPool) && this->_servers_map[i].getServerSocket() == fd && !clientRegistred){
                    this->registerNewConnections(fd, &this->_servers_map[i]);
                    clientRegistred = true;
                }
            }
            if (FD_ISSET(fd, &this->_readPool) && this->_clients_map.count(fd) > 0){
                this->processRequest(this->_clients_map[fd]);
            }
            else if (FD_ISSET(fd, &this->_writePool) && this->_clients_map.count(fd) > 0){
                Cgi& cgi_obj = this->_clients_map[fd]->getCgiObj();
                int cgi_state = cgi_obj.getCGIState();
                if (cgi_state == 1 && FD_ISSET(cgi_obj.getPipeIn()[1], &this->_writePool))
                    sendCgiBody(fd, _clients_map[fd], cgi_obj);
                else if (cgi_state == 1 && FD_ISSET(cgi_obj.getPipeOut()[0], &this->_readPool))
                    readCgiBody(fd, _clients_map[fd], cgi_obj);
                else if ((cgi_state == 0 || cgi_state == 2) && FD_ISSET(fd, &this->_writePool))
                    this->sendResponse(fd, this->_clients_map[fd]);
            }
        }
    }
    return;
}

void ServerManager::sendCgiBody(SOCKET fd, Client *client, Cgi &cgi_obj)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();
    if (!request || !response)
    {
        Logger::error("ServerManager", "Invalid request or response state [" + intToStr(fd) + "]");
        return;
    }

    size_t total = request->getBody().length();
    size_t sent = cgi_obj.getBytesSended();
    if (sent >= total)
    {
        removeFromSet(cgi_obj.getPipeIn()[1], &this->_writePool);
        close(cgi_obj.getPipeIn()[1]);
        return;
    }
    Logger::info("Sending body to CGI process");
    ssize_t bytes_sent = write(cgi_obj.getPipeIn()[1], request->getBody().c_str() + sent, total - sent);
    if (bytes_sent < 0)
    {
        Logger::error(__FILE__, "sendCgiBody() Error while sending CGI data to CGI process");
        removeFromSet(cgi_obj.getPipeIn()[1], &this->_writePool);
        close(cgi_obj.getPipeIn()[1]);
        response->setStatusCode(500);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
    }
    else
    {
        cgi_obj.incrementBytesSended(bytes_sent);
        if (cgi_obj.getBytesSended() == total)
        {
            Logger::info("Body fully sent to CGI Process.");
            removeFromSet(cgi_obj.getPipeIn()[1], &this->_writePool);
            close(cgi_obj.getPipeIn()[1]);
        }
    }
}

void ServerManager::readCgiBody(SOCKET fd, Client *client, Cgi &cgi_obj)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();
    if (!request || !response)
    {
        Logger::error("ServerManager", "Invalid request or response state [" + intToStr(fd) + "]");
        return;
    }
    Logger::info("About to read CGI response: ");
    char buffer[BUFFER_SIZE + 1] = {};
    int bytes_read = read(cgi_obj.getPipeOut()[0], buffer, BUFFER_SIZE);
    Logger::info("Bytes read from CGI: [" + intToStr(bytes_read) + std::string("]"));
    if (bytes_read < 0)
    {
        Logger::error(__FILE__, "readCgiBody() Error while reading CGI data from CGI process");
        removeFromSet(cgi_obj.getPipeOut()[0], &this->_readPool);
        close(cgi_obj.getPipeOut()[0]);
        response->setStatusCode(500);
        response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        return;
    }
    else if (bytes_read == 0)
    {
        Logger::info("Received EOF from CGI:" + response->getBody());
        removeFromSet(cgi_obj.getPipeOut()[0], &this->_readPool);
        close(cgi_obj.getPipeOut()[0]);
        cgi_obj.setCGIState(2);
    }
    else
    {
        response->setBody(response->getBody() + std::string(buffer));
        Logger::info("Chunk of CGI response: " + std::string(buffer));
    }
}

void ServerManager::registerNewConnections(SOCKET serverFd, Server *server)
{
    Client *client = this->getClient(-1);
    if (client == NULL){ 
        return;
    }
    SOCKET new_socket = accept(serverFd, (sockaddr *)&client->getAddr(), &client->getAddrLen());
    if (new_socket < 0){
        delete client;
        return;
    }
    if (fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0){
        Logger::error("ServerManager", "Error: fcntl failed");
        delete client;
        close(new_socket);
        return;
    }

    Logger::info("New connection from " + this->getClientIP(client) + " [" + intToStr(new_socket) + "]");
    client->setSocketFd(new_socket);
    this->addToSet(new_socket, &this->_masterPool);
    client->setServer(server);
    client->updateLastActivity();
    this->_clients_map[new_socket] = client;
}

void ServerManager::processRequest(Client *client)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();

    char buffer[BUFFER_SIZE + 1];
    memset(buffer, 0, sizeof(buffer));

    const SOCKET fd = client->getSocketFd();
    const int bytesRecv = recv(fd, buffer, BUFFER_SIZE, 0);
    if (bytesRecv == -1){
        Logger::error("ServerManager", "Error in recv(): " + std::string(strerror(errno)) + " [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
        return;
    }

    if (bytesRecv == 0){
        Logger::info("Client closed connection [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
        return;
    }

    if (bytesRecv > 0){
        Logger::info("Received " + intToStr(bytesRecv) + " bytes [" + intToStr(fd) + "]");
        std::string str(buffer, static_cast<std::string::size_type>(bytesRecv));
        request->consume(str);
    }

    if (request->getHasBody() && request->getBodyCounter() > MAX_REQUEST_SIZE){
        Logger::error("ServerManager", "Request body too large [" + intToStr(fd) + "]");
        response->setStatusCode(413);
        request->setState(StateParsingError);
        return;
    }

    if (request->getState() == StateParsingComplete || request->getState() == StateParsingError){
        Logger::info("Request was consumed assigning server [" + intToStr(fd) + "]");
        this->assignServer(client);
        if (request->getState() == StateParsingComplete){
             Logger::info("Request parsing complete, validating resource [" + intToStr(fd) + "]");
            Parser parser;
            parser.validateResource(client, client->getServer());
            if (response->getStatus() != 200){
                this->removeFromSet(fd, &this->_readPool);
                this->addToSet(fd, &this->_writePool);
                return;
            }
        }
        if (client->getCgiObj().getCGIState() == 1 && request->getState() == StateParsingComplete){
            Logger::info("Request is a CGI request [" + intToStr(fd) + "]");
            client->setIsCgiRequest(true);
            this->addToSet(client->getCgiObj().getPipeIn()[1], &this->_writePool);
            this->addToSet(client->getCgiObj().getPipeOut()[0], &this->_readPool);
        }
        if (request->getState() == StateParsingError){
            Logger::error("ServerManager", "Error parsing request [" + intToStr(fd) + "]");
            response->setStatusCode(request->getError());
            response->setBody(getErrorPage(response->getStatus(), client->getServer()));
        }
        this->removeFromSet(fd, &this->_readPool);
        this->addToSet(fd, &this->_writePool);
    }

    client->updateLastActivity();
}

void ServerManager::sendResponse(SOCKET fd, Client *client)
{
    Request *request = client->getRequest();
    Response *response = client->getResponse();
    client->updateLastActivity();

    if (!request || !response){
        Logger::error("ServerManager", "Invalid request or response state [" + intToStr(fd) + "]");
        return;
    }

    response->setHeaders("Host", "localhost");
    std::string connectionHeader = to_lower(request->getHeaders()["connection"]);
    connectionHeader == "close" ? response->setHeaders("Connection", "close") : response->setHeaders("Connection", "keep-alive");
    if (!response->getBody().empty()){
        response->setHeaders("Content-Type", getContentType(request->getUrl(), response->getStatus()));
        response->setHeaders("Content-Length", intToStr(response->getBody().size()));
    } else {
        response->setHeaders("Content-Type", "text/html");
        response->setHeaders("Content-Length", "0");
    }
    response->prepareResponse();
    int bytes_sent = send(fd, response->getResponse().c_str(), response->getResponse().size(), 0);
    if (bytes_sent == -1){
        Logger::error("ServerManager", "Send failed: " + std::string(strerror(errno)) + " [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
        return;
    }
    Logger::info("Response sent successfully (" + intToStr(bytes_sent) + " bytes) [" + intToStr(fd) + "]");
    if (connectionHeader == "close"){
        Logger::info("Closing connection as requested by client [" + intToStr(fd) + "]");
        this->closeClientConnection(fd, client);
    } else {
        client->reset();
        this->closeClientConnection(fd, client);
        Logger::info("Connection kept alive for next request [" + intToStr(fd) + "]");
    }
}
