#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"



//Response  Section 6
// CRLF = "\r\n"
// Response      =  Status-Line               ; Section 6.1
//                  *(( general-header        ; Section 4.5
//                   | response-header        ; Section 6.2
//                   | entity-header ) CRLF)  ; Section 7.1
//                  CRLF
//                  [ message-body ]          ; Section 7.2

void Response::print(){
    std::cout << "Response: " << std::endl;
    std::cout << this->_finalResponse << std::endl;
}

void Response::prepareResponse(){
    
    this->fillStatusLine();
    this->_finalResponse.append("\r\n");

    std::map<std::string, std::string>::iterator headerMapIt;
    
    for(headerMapIt = this->_headers.begin(); headerMapIt != this->_headers.end(); headerMapIt++){
        this->fillHeader(headerMapIt->first, headerMapIt->second);
    }
    this->_finalResponse.append("\r\n");
    
    
    this->_finalResponse.append(this->_body);
}

void Response::fillStatusLine(){
    this->_finalResponse.append("HTTP/1.1 ");
    this->_finalResponse.append(intToStr(this->_status));
    this->_finalResponse.append(" ");
    this->_finalResponse.append(getMessageFromStatusCode(this->_status));
}

void Response::fillHeader(std::string headerKey, std::string headerValue){
    this->_finalResponse.append(headerKey);
    this->_finalResponse.append(": ");
    this->_finalResponse.append(headerValue);
    this->_finalResponse.append("\r\n");    
}

void Response::reset(void) {
    this->_finalResponse.clear();
    this->_headers.clear();
    this->_body.clear();
    this->_content_type.clear();
    this->_status_message = "OK";
    this->_status = 200;
}

void Response::setHeaders(std::string key, std::string value) {this->_headers[key] = value;}

std::string &Response::getResponse()    {return this->_finalResponse;}
std::string &Response::getContentType() {return this->_content_type;};
std::string &Response::getBody()    {return this->_body;}
std::string &Response::getStatusMessage()   {return this->_status_message;}
std::map<std::string, std::string> &Response::getHeaders()  {return this->_headers;}

int &Response::getStatus()  {return this->_status;};

void Response::setResponse(char *response)  {this->_finalResponse = response;}
void Response::setBody(std::string body)    {this->_body = body;}
void Response::setContentType(std::string content_type) {this->_content_type = content_type;}
void Response::setStatusMessage(std::string status_message) {this->_status_message = status_message;}
void Response::setStatusCode(int status)    {this->_status = status;}

void Response::flush(){

   this->_finalResponse = "";
   this->_headers.clear();
   this->_body = "";
   this->_content_type = ""; 
   this->_status_message = "";  
   this->_status = 0;
};


Response::Response()
{
    this->_finalResponse = "";
    this->_body = "";
    this->_content_type = "";
    this->_status_message = "OK";
    this->_status = 200;
}

Response::Response(int status, const char *status_message)
{
    this->_finalResponse = "";
    this->_body = "";
    this->_content_type = "";
    this->_status_message = status_message;
    this->_status = status;
}


Response::Response(int status, std::string status_message)
{
    this->_finalResponse = "";
    this->_status = status;
    this->_status_message = status_message;
    this->_body = "";
    this->_content_type = "";
}

Response::~Response()
{
    Logger::info("Response Destructor() Called!!!");

}
