#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>


class Request {
public:
    Request();
    ~Request();

    // Request consumption
    int  consume(const std::string& buffer);
    
    //debug
    void print();

    // Getters
    std::string& getMethod();
    std::string& getBody();
    std::string& getUrl();
    std::string& getVersion();
    std::map<std::string, std::string>& getHeaders();
    std::string& getQueryParam();
    int getState();
    int getError() const;
    int getBodyCounter() const;
    bool getHasBody() const;

    // Setters
    void setQueryParam(std::string& queryParam);
    void setError(int errorCode);
    void setUrl(std::string& url);
    void setVersion(std::string& version);
    void setHeaders(std::map<std::string, std::string>& headers);
    void setBody(std::string& body);
    void setHasBody(bool hasBody);
    void setState(int state);
    void setBodyCounter(int counter);
    void reset();

private:
    // Basic req components
    std::string method;
    std::string url;
    std::string version;
    std::string query_param;
    std::map<std::string, std::string> headers;
    std::string raw;
    std::string body;

    // Internal parsing/processing state
    std::string content;
    int state;
    bool has_body;
    bool is_chunked;
    int error;
    int body_counter;

    size_t number;
    size_t encoded_counter;
    std::string encoded_char;
    std::string headers_key;

    std::map<int, std::string> methods;
};

#endif // REQUEST_HPP
