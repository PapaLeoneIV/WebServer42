#ifndef SERVER_HPP
#define SERVER_HPP


#include <sstream>
#include <sys/types.h>
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
    SOCKET      getServerSocket() const;
    fd_set      getFdsSet() const;
    addrinfo    &getHints();
    addrinfo    *getBindAddress() const;
    std::string &gethostPortPair();
    std::map<std::string, std::vector<std::string> > &getServerDir();
    std::map<std::string, std::map<std::string, std::vector<std::string> > > &getLocationDir(); 

    //SETTERS
    void    setServerDir(std::string key, std::vector<std::string> value);
    void    setLocationDir(const std::string& locationPath, const std::string& key,  const std::vector<std::string>& value);

    void    setServerSocket(SOCKET server_socket);
    void    setFds(const fd_set &fds);
    void    setHints(const addrinfo &hints);
    void    setBindAddress(addrinfo *bind_address);
    void    sethostPortPair(const std::string& hostPortPair);

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