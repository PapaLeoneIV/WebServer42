#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Cgi.hpp"
#include "../includes/Logger.hpp"

time_t Client::getLastActivity() const {return this->_last_activity;}

void Client::updateLastActivity() {this->_last_activity = time(NULL);}

Cgi &Client::getCgiObj() {return this->cgi_obj;}

sockaddr_storage &Client::getAddr(){return this->address;};

socklen_t   &Client::getAddrLen() {return this->addressLength;};

SOCKET  &Client::getSocketFd() {return this->socket;};

Request *Client::getRequest() const {return this->req;}

Response    *Client::getResponse() const {return this->resp;}

Server *Client::getServer() const {return this->s;}

void Client::setIsCgiRequest(const bool value){this->isCGIRequest = value;}

void    Client::setAddr(const sockaddr_storage &addr) {this->address = addr;}

void    Client::setAddrLen(const socklen_t len)  {this->addressLength = len;}

void    Client::setSocketFd(const SOCKET fd) {this->socket = fd;}


void    Client::setServer(Server *s)  {this->s = s;}

void Client::reset() {
    if (this->req) {this->req->reset();}
    if (this->resp) {this->resp->reset();}
    this->_received = 0;
    this->_content_length = 0;
    this->updateLastActivity();
}

Client::Client() : address(), _content_length() {
    this->req = new Request();
    this->resp = new Response();
    this->s = NULL;
    memset(&this->address, 0, sizeof(this->address));
    this->addressLength = sizeof(this->address);
    this->socket = -1;
    this->_received = 0;
    this->_last_activity = time(NULL);
    this->isCGIRequest = false;
};




Client::~Client(){
  static char address_info[INET6_ADDRSTRLEN];
  const int error = getnameinfo(reinterpret_cast<sockaddr *>(&this->getAddr()), this->getAddrLen(), address_info, sizeof(address_info), NULL, 0, NI_NUMERICHOST);
  if (error) {
    Logger::error("Client", "Error getting c address: " + std::string(gai_strerror(error)));
  } else {
    Logger::info("Connection closed from " + std::string(address_info) +  printFd(this->getSocketFd()));
  }
    delete this->req;
    delete this->resp;
};