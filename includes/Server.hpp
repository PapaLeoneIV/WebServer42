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
    std::string &gethostPortPair();
    std::map<std::string, std::vector<std::string> > &getServerDir();
    std::map<std::string, std::map<std::string, std::vector<std::string> > > &getLocationDir(); 

    //SETTERS
    void    setServerDir(std::string key, std::vector<std::string> value);
    void    setLocationDir(std::string locationPath, std::string key,  std::vector<std::string> value);

    void    setServerSocket(SOCKET server_socket);
    void    setFds(fd_set fds);
    void    setHints(addrinfo hints);
    void    setBindAddress(addrinfo *bind_address);
    void    sethostPortPair(std::string hostPortPair);

    private:
    std::map<std::string, std::vector<std::string> > serverDir;
    std::map<std::string, std::map<std::string, std::vector<std::string> > > locationDir;
    
    std::string hostPortPair;
    //Location configuration

    addrinfo    hints;
    SOCKET      serverSocket;
    fd_set      fds;
    addrinfo    *bindAddress;

};

#endif