#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <iostream>
#include "Client.hpp"
typedef int SOCKET;

class Response {
public:
    // Constructors & Destructor
    Response();
    ~Response();

    // Core Methods
    void print() const;
    void prepareResponse(Client *c, Request *req);
    void fillStatusLine();
    void fillHeader(const std::string& headerKey, const std::string& headerValue);

    // Getters
    std::string& getResponse();
    std::string& getBody();
    std::string& getStatusMessage();
    int& getStatus();
    std::map<std::string, std::string>& getHeaders();

    // Setters
    void setResponse(const char* resp);
    void setBody(const std::string& body);
    void setStatusMessage(const std::string& status_message);
    void setStatusCode(int status);
    void setHeaders(std::string key, std::string value);

    // Utilities
    void reset();

private:
    std::string                         finalResponse;
    std::map<std::string, std::string>  headers;
    std::string                         body;
    std::string                         contentType;
    std::string                         statusMessage;
    int                                 status;
};

#endif // RESPONSE_HPP
