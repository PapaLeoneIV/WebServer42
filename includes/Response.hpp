#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

typedef int SOCKET;
class Response{
    
    public:

    Response();
    Response(int status, const char *status_message);
    Response(int status, std::string status_message);
    ~Response();

    void print();
    void prepareResponse();
    void fillStatusLine();
    void fillHeader  (std::string headerKey, std::string headerValue);
    
    //GETTERS
    std::string &getResponse();
    std::string &getBody();
    std::string &getContentType();
    std::string &getStatusMessage();
    int  &getStatus();
    std::map<std::string, std::string>  &getHeaders();
    
    //SETTERS
    void setResponse (char *response);
    void setBody (std::string body);
    void setContentType(std::string content_type);
    void setStatusMessage(std::string status_message);
    void setStatusCode (int status);
    void setHeaders(std::string key, std::string value);
    void reset();
    void flush();

    private:

    std::string                         _finalResponse;
    std::map<std::string, std::string>  _headers;
    std::string                         _body;
    std::string                         _content_type; 
    std::string                         _status_message; 
    int                                 _status;
};


#endif
