#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

typedef int SOCKET;

class Response {
public:
    // Constructors & Destructor
    Response();
    Response(int status, const char* status_message);
    Response(int status, const std::string& status_message);
    ~Response();

    // Core Methods
    void print();
    void prepareResponse();
    void fillStatusLine();
    void fillHeader(const std::string& headerKey, const std::string& headerValue);

    // Getters
    std::string& getResponse();
    std::string& getBody();
    std::string& getContentType();
    std::string& getStatusMessage();
    int& getStatus();
    std::map<std::string, std::string>& getHeaders();

    // Setters
    void setResponse(const char* resp);
    void setBody(const std::string& body);
    void setContentType(const std::string& content_type);
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
