#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>

class Server;

#define BUFFER_SIZE (4*1024) //4KB

#define MAX_REQUEST_SIZE (100*1024*1024) //100MB

#define TIMEOUT_SEC 20

#define VERSION "4.2.0"

//List of possible error in the program, used to return a string error message
//to check the actual error message, checkout function ErrToStr in Utils.cpp
enum POSSIBLE_ERRORS{
    FAILURE = -1,
    SUCCESS, 
    //BOOTING ERRORS
    ERR_RESOLVE_ADDR,
    ERR_SOCK_CREATION,
    ERR_SOCKET_NBLOCK,
    ERR_BIND,
    ERR_LISTEN,
    ERR_FCNTL,
    //MONITOR ERRORS
    ERR_SELECT,
    ERR_SEND,
    ERR_RECV,
    //PARSER ERRORS
    INVALID_METHOD,
    INVALID_URL,
    INVALID_VERSION,
    INVALID_MANDATORY_HEADER,
    INVALID_BODY,
    INVALID_BODY_LENGTH,
    INVALID_MAX_REQUEST_SIZE,
    INVALID_CONNECTION_CLOSE_BY_CLIENT,
    INVALID_REQUEST,
    MISSING_HEADER,
    INVALID_CONTENT_LENGTH,
    FILE_NOT_FOUND,
    FILE_READ_DENIED,
    INVALID_HEADER,
};


enum RequestStates {
    StateMethod,                                //0
    StatePostOrPut,                             //1

    StateSpaceAfterMethod,                      //2
    StateUrlBegin,                              //3
    StateUrlString,                             //4
    StateUrlQuery,                              //5
    StateEncodedSep,                            //6
    StateSpaceAfterUrl,                         //7

    StateVersion,                               //8
    StateVersion_H,                             //9
    StateVersion_HT,                            //10
    StateVersion_HTT,                           //11
    StateVersion_HTTP,                          //12
    StateVersion_HTTPSlash,                     //13
    StateVersion_HTTPSlashOne,                  //14
    StateVersion_HTTPSlashOneDot,               //15
    StateVersion_HTTPSlashOneDotOne,            //16
    StateFirstLine_CR,                          //17
    StateFirstLine_LF,                          //18
    
    StateHeaderKey,                             //19
    StateHeadersTrailingSpaceStart,             //20
    StateHeaderValue,                           //21
    StateHeadersTrailingSpaceEnd,               //22
    StateHeaders_CR,                            //23
    StateHeaders_LF,                            //24
    StateHeadersEnd_CR,                         //25 
    
    
    StateBodyStart,                             //26
    StateBodyPlainText,                         //27
    StateBodyBodyChunkedText,                   //28
    StateBodyChunkedNumber,                     //29
    StateBodyEnd_CR,                            //30
    StateBodyEnd_LF,                            //31
    StateChunkedEnd_LF,                         //32
    StateChunkedNumber_LF,                      //33
    StateChunkedNumber_CR,                      //33
    StateChunkedChunk_LF,                       //34
    StateChunkedChunk,                          //35
    
    StateParsingComplete,                       //36
    StateParsingError,                          //37
  
  };
  
  enum RequestMethods{
    GET,
    POST,
    DELETE,
  };

std::string printFd(int fd);

int handle_arguments(int argc , char **argv);

std::string to_lower(const std::string& input);

int wb_stox(const std::string& str);

std::string wb_itos(int number);

int wb_stoi(const std::string& str);

std::string wb_ltos(const long number);

std::string fromDIRtoHTML(const std::string& path, const std::string& url);

std::string readTextFile(const std::string& path);

std::string fromCodeToMsg(int status);

std::string getContentType(const std::string& url, int status);

std::string ErrToStr(int error);

std::string getErrorPage(int status, Server *s);
#endif