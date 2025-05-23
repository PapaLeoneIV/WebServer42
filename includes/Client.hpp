#ifndef CLIENTS_HPP
#define CLIENTS_HPP


#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include "Cgi.hpp"
class Server;
class Request;
class Response;
class Cgi;

typedef int SOCKET;

class Client{

    public:

    Client();
    Client(SOCKET fd);

    ~Client();
    
    //GETTERS
    Cgi &getCgiObj();
    Response    *getResponse();
    Request *getRequest();
    sockaddr_storage    &getAddr();
    socklen_t   &getAddrLen();
    SOCKET  &getSocketFd();
    std::string     getRequestData();
    int     &getRecvBytes();
    Server*     getServer(); 
    std::string     getHeadersData();
    std::string     getBodyData();
    int     getContentLength();
    time_t  getLastActivity();
    bool isCgiRequest();
    //SETTERS
    void setCgiObj(Cgi cgi_obj);
    void    set_Request(Request *request);
    void    set_Response(Response *response);
    void    setAddr(sockaddr_storage addr); 
    void    setAddrLen(socklen_t len);
    void    setSocketFd(SOCKET fd);
    void    setRequestData(std::string requestData);
    void    setRecvData(int bytes);
    void    setServer(Server *server);
    void    setHeadersData(std::string headersData);
    void    setBodyData(std::string bodyData);
    void    setContentLength(int content_length); 
    void    updateLastActivity(void);
    void    reset(void);
    void    setIsCgiRequest(bool innit);
                            
    private:
    Request *_Request;
    Response    *_Response;
    Cgi _cgi_obj;

    Server  *_server;
    
    sockaddr_storage    _address;
    socklen_t   _address_length;
    SOCKET  _socket;
    std::string _requestData;
    std::string _headersData;
    std::string _bodyData; 
    int _content_length;
    int _received;
    time_t  _last_activity;
    bool isCGIRequest;
};

#endif
