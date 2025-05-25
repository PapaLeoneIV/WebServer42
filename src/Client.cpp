#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Cgi.hpp"
#include "../includes/Logger.hpp"

time_t Client::getLastActivity(void) {return this->_last_activity;}

void Client::updateLastActivity(void) {this->_last_activity = time(NULL);}

Cgi &Client::getCgiObj() {return this->_cgi_obj;}

sockaddr_storage &Client::getAddr(){return this->_address;};

socklen_t   &Client::getAddrLen() {return this->_address_length;};

SOCKET  &Client::getSocketFd() {return this->_socket;};

Request *Client::getRequest() {return this->_Request;}

Response    *Client::getResponse() {return this->_Response;}

std::string Client::getRequestData() {return this->_requestData;}

int &Client::getRecvBytes() {return this->_received;}

Server *    Client::getServer() {return this->_server;}

std::string Client::getHeadersData() {return this->headersData;}

std::string Client::getBodyData() {return this->_bodyData;}

int Client::getContentLength() {return this->_content_length;}

void Client::setIsCgiRequest(bool innit){this->isCGIRequest = innit;}

void Client::setCgiObj(Cgi cgi_obj) {this->_cgi_obj = cgi_obj;}

void    Client::setHeadersData(const std::string &headersData) {this->headersData = headersData;}

void    Client::setBodyData(const std::string &bodyData) {this->_bodyData = bodyData;}

void    Client::setAddr(sockaddr_storage addr) {this->_address = addr;}

void    Client::setAddrLen(socklen_t len)  {this->_address_length = len;}

void    Client::setSocketFd(SOCKET fd) {this->_socket = fd;}

void    Client::set_Request(class Request *_Request){this->_Request = _Request;}

void    Client::set_Response(class Response *_Response){this->_Response = _Response;} 

void    Client::setRequestData(const std::string& requestData){this->_requestData = requestData;}

void    Client::setRecvData(int bytes) {this->_received = bytes;}

void    Client::setServer(Server *server)  {this->_server = server;}

void    Client::setContentLength(int content_length) {this->_content_length = content_length;}

bool    Client::isCgiRequest(){return this->isCGIRequest;}

void Client::reset() {
    if (this->_Request) {
        this->_Request->reset();
    }
    if (this->_Response) {
        this->_Response->reset();
    }
    
    this->_requestData = "";
    this->headersData = "";
    this->_bodyData = "";
    this->_received = 0;
    this->_content_length = 0;
    
    this->updateLastActivity();
    
    Logger::info("Client state reset for keep-alive connection [" + wb_itos(this->_socket) + "]");
}

Client::Client() {
    this->_Request = new Request();
    this->_Response = new Response();
    this->_server = NULL;
    memset(&this->_address, 0, sizeof(this->_address));
    this->_address_length = sizeof(this->_address);
    this->_socket = -1;
    this->_requestData = "";
    this->headersData = "";
    this->_bodyData = "";
    this->_received = 0;
    this->_last_activity = time(NULL);
    this->isCGIRequest = 0;
};


Client::~Client(){
  static char address_info[INET6_ADDRSTRLEN];
  int x = getnameinfo((sockaddr*)&this->getAddr(), this->getAddrLen(), address_info, sizeof(address_info), NULL, 0, NI_NUMERICHOST);
  if (x != 0) {
    Logger::error("Client", "Error getting c address: " + std::string(gai_strerror(x)));
  } else {
    Logger::info("Connection closed from " + std::string(address_info) + " [" + wb_itos(this->getSocketFd()) + "]");
  }
  if(this->_Request)
    delete this->_Request;
  if(this->_Response)
    delete this->_Response;
};