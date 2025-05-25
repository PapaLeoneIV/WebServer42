#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <ctime>

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
    Cgi&            getCgiObj();
    Request*        getRequest();
    Response*       getResponse();
    sockaddr_storage& getAddr();
    socklen_t&      getAddrLen();
    SOCKET&         getSocketFd();
    std::string     getRequestData();
    std::string     getHeadersData();
    std::string     getBodyData();
    int             getContentLength();
    int&            getRecvBytes();
    Server*         getServer();
    time_t          getLastActivity();
    bool            isCgiRequest();

    // Setters
    void            setCgiObj(Cgi cgi_obj);
    void            set_Request(Request* req);
    void            set_Response(Response* resp);
    void            setAddr(sockaddr_storage addr); 
    void            setAddrLen(socklen_t len);
    void            setSocketFd(SOCKET fd);
    void            setRequestData(const std::string& requestData);
    void            setHeadersData(const std::string& headersData);
    void            setBodyData(const std::string& bodyData);
    void            setContentLength(int contentLength); 
    void            setRecvData(int bytes);
    void            setServer(Server* server);
    void            updateLastActivity();
    void            setIsCgiRequest(bool value);
    void            reset();

private:
    // Request handling
    Request*        _Request;
    Response*       _Response;
    Cgi             _cgi_obj;

    // Connection metadata
    Server*             _server;
    sockaddr_storage    _address;
    socklen_t           _address_length;
    SOCKET              _socket;

    // Request data
    std::string     _requestData;
    std::string     headersData;
    std::string     _bodyData;

    // Request state
    int             _content_length;
    int             _received;
    bool            isCGIRequest;

    // Activity tracking
    time_t          _last_activity;
};

#endif // CLIENTS_HPP
