#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"



//Response  Section 6
// CRLF = "\r\n"
// Response      =  Status-Line               ; Section 6.1
//                  *(( general-header        ; Section 4.5
//                   | resp-header        ; Section 6.2
//                   | entity-header ) CRLF)  ; Section 7.1
//                  CRLF
//                  [ message-body ]          ; Section 7.2

void Response::print(){
    std::cout << "Response: " << std::endl;
    std::cout << this->finalResponse << std::endl;
}

void Response::prepareResponse(){
    
    this->fillStatusLine();
    this->finalResponse.append("\r\n");

    std::map<std::string, std::string>::iterator headerMapIt;
    
    for(headerMapIt = this->headers.begin(); headerMapIt != this->headers.end(); headerMapIt++){
        this->fillHeader(headerMapIt->first, headerMapIt->second);
    }
    this->finalResponse.append("\r\n");
    this->finalResponse.append(this->body);
}

void Response::fillStatusLine(){
    this->finalResponse.append("HTTP/1.1 ");
    this->finalResponse.append(wb_itos(this->status));
    this->finalResponse.append(" ");
    this->finalResponse.append(getMessageFromStatusCode(this->status));
}

void Response::fillHeader(const std::string& headerKey, const std::string& headerValue){
    this->finalResponse.append(headerKey);
    this->finalResponse.append(": ");
    this->finalResponse.append(headerValue);
    this->finalResponse.append("\r\n");    
}

void Response::reset(void) {
    this->finalResponse.clear();
    this->headers.clear();
    this->body.clear();
    this->contentType.clear();
    this->statusMessage = "OK";
    this->status = 200;
}

void Response::setHeaders(std::string key, std::string value) {this->headers[key] = value;}

std::string &Response::getResponse()    {return this->finalResponse;}
std::string &Response::getContentType() {return this->contentType;};
std::string &Response::getBody()    {return this->body;}
std::string &Response::getStatusMessage()   {return this->statusMessage;}
std::map<std::string, std::string> &Response::getHeaders()  {return this->headers;}

int &Response::getStatus()  {return this->status;};

void Response::setResponse(const char *resp)  {this->finalResponse = resp;}
void Response::setBody(const std::string& body)    {this->body = body;}
void Response::setContentType(const std::string& content_type) {this->contentType = content_type;}
void Response::setStatusMessage(const std::string& status_message) {this->statusMessage = status_message;}
void Response::setStatusCode(int status)    {this->status = status;}




Response::Response()
{
    this->finalResponse = "";
    this->body = "";
    this->contentType = "";
    this->statusMessage = "OK";
    this->status = 200;
}

Response::Response(int status, const char *status_message)
{
    this->finalResponse = "";
    this->body = "";
    this->contentType = "";
    this->statusMessage = status_message;
    this->status = status;
}


Response::Response(int status, const std::string& status_message)
{
    this->finalResponse = "";
    this->status = status;
    this->statusMessage = status_message;
    this->body = "";
    this->contentType = "";
}

Response::~Response(){}
