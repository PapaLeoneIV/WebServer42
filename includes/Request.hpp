#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

typedef int SOCKET;

/**Request class, contains the data rappresentation that an incoming request might contain*/
class Request{

public:
    
    Request();
    ~Request();

    int  consume(std::string buffer);
    void print_Request();
    void  printHeaders();
    
    //GETTERS
    std::string &getMethod();
    std::string &getBody();
    int getState();
    std::string &getUrl();
    std::string &getVersion();
    int getError();
    std::map<std::string, std::string>  &getHeaders();
    int getBodyCounter();
    std::string &getQueryParam();
    std::string getRaw();
    bool getHasBody();

    //SETTERS
    void setQueryParam(std::string& query_param);
    void  setError(int error);
    void  setUrl(std::string& url);
    void  setVersion(std::string& version);
    void  setHeaders(std::map<std::string, std::string>& headers);
    void  setBody(std::string& body);
    void  setHasBody(bool hasBody);
    void  setState(int state);
    void setBodyCounter(int bodyCounter);
    void  flush();
    bool  &hasBody();
    void reset();

    
    private:
    std::string url;
    std::string version;
    std::string method;
    std::map<std::string, std::string> headers;
    std::string query_param;
    std::string raw;
    int state;
    
    std::string body;
    std::string content;
    bool has_body;
    bool is_chunked;
    int error;
    bool MultiPartHeaderFound;
    
    size_t number;
    int body_counter;
    size_t encoded_counter;
    std::string encoded_char;
    std::string headers_key;
    std::map<int, std::string> methods;
    
};

#endif 
