#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include "Cgi.hpp"

class Server;
class Request;
class Response;

typedef int SOCKET;

class Client {
public:
    // Constructors & Destructor
    Client();
    ~Client();

    // Getters
    Cgi&    getCgiObj();
    Request*    getRequest() const;
    Response*   getResponse() const;
    sockaddr_storage& getAddr();
    socklen_t&  getAddrLen();
    SOCKET& getSocketFd();
    Server* getServer() const;
    time_t  getLastActivity() const;

    // Setters
    void    setAddr(const sockaddr_storage &addr); 
    void    setAddrLen(socklen_t len);
    void    setSocketFd(SOCKET fd);
    void    setServer(Server* s);
    void    updateLastActivity();
    void    setIsCgiRequest(bool value);
    void    reset();

private:
    // Request handling
    Request*        req;
    Response*       resp;
    Cgi             cgi_obj;

    // Connection metadata
    Server*             s;
    sockaddr_storage    address;
    socklen_t           addressLength;
    SOCKET              socket;

    // Request state
    int             _content_length;
    int             _received;
    bool            isCGIRequest;

    // Activity tracking
    time_t          _last_activity;
};

#endif // CLIENTS_HPP
