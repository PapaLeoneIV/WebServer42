#ifndef SERVER_HPP
#define SERVER_HPP


#include <string.h>
#include <unistd.h>
#include <sstream>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <string>
#include <map>
#include <vector>



typedef int SOCKET;
typedef int ERROR;

class Server{

    public:    
    Server();
    ~Server();

    // GETTERS
    SOCKET      getServerSocket();
    fd_set      getFdsSet();
    addrinfo    &getHints();
    addrinfo    *getBindAddrss();
    std::string &getHost();
    std::string &getServerName();
    std::string &getPort();
    std::string &getRoot();
    std::string &getIndex();
    std::string &getHostPortKey();
    size_t      &getMaxRequestSize();
    std::map<std::string, std::vector<std::string> > &getServerDir();
    std::map<std::string, std::map<std::string, std::vector<std::string> > > &getLocationDir(); 

    //SETTERS
    void    setServerDir(std::string key, std::vector<std::string> value);
    void    setLocationDir(std::string locationPath, std::string key,  std::vector<std::string> value);

    void    setServerSocket(SOCKET server_socket);
    void    setFds(fd_set fds);
    void    setHints(addrinfo hints);
    void    setBindAddress(addrinfo *bind_address);
    void    setHost(std::string host);
    void    setServerName(std::string server_name);
    void    setPort(std::string port);
    void    setRoot(std::string root);
    void    setIndex(std::string index);
    void    setHostPortKey(std::string hostPortKey);
    void    setMaxRequestSize(size_t max_request_size);

    void printServerDir(void);
    void printLocationDir(void);

    private:

    std::map<std::string, std::vector<std::string> > serverDir;
    std::map<std::string, std::map<std::string, std::vector<std::string> > > locationDir;
    
    std::string _host;
    std::string _server_name;
    std::string _port;
    std::string _root;
    std::string _index;

    std::string hostPortKey;
    //Location configuration

    addrinfo    _hints;
    SOCKET      _server_socket;
    fd_set      _fds;
    addrinfo    *_bind_address;

};

#endif