#include "../includes/Logger.hpp"
#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"

//GETTERS
std::string &Server::getHostPortKey() {return this->hostPortKey;};
SOCKET      Server::getServerSocket()	{return this->_server_socket;}
fd_set      Server::getFdsSet()	{return this->_fds;}
addrinfo    &Server::getHints()	{return this->_hints;}
addrinfo    *Server::getBindAddrss()	{return this->_bind_address;}
std::string &Server::getHost()	{return this->_host;};
std::string &Server::getServerName()	{return this->_server_name;};
std::string &Server::getPort()	{return this->_port;};
std::string &Server::getRoot()	{return this->_root;};
std::string &Server::getIndex()	{return this->_index;};
std::map<std::string, std::vector<std::string> > &Server::getServerDir(void) {return this->serverDir;}
std::map<std::string, std::map<std::string, std::vector<std::string> > > &Server::getLocationDir(void) {return this->locationDir;} 


//SETTERS
void    Server::setHostPortKey(std::string hostPortKey){this->hostPortKey = hostPortKey;};
void    Server::setServerSocket(SOCKET server_socket)   {this->_server_socket = server_socket;};
void    Server::setFds(fd_set fds)	{this->_fds = fds;};
void    Server::setHints(addrinfo hints) {this->_hints = hints;};
void    Server::setBindAddress(addrinfo *bind_address)  {this->_bind_address = bind_address;};
void    Server::setHost(std::string host) {this->_host = host;};
void    Server::setServerName(std::string server_name)  {this->_server_name = server_name;};
void    Server::setPort(std::string port) {this->_port = port;};
void    Server::setRoot(std::string root) {this->_root = root;};
void    Server::setIndex(std::string index) {this->_index = index;};

void	Server::setServerDir(std::string key, std::vector<std::string> value) {
	if(key == "error_page" && value.size() > 1)
	{
		key.append("_");
		key.append(value[0]);
		value.erase(value.begin());
	}
	this->serverDir[key] = value;
}

void	Server::setLocationDir(std::string locationPath, std::string key,  std::vector<std::string> value) {this->locationDir[locationPath][key] = value;}

void Server::printServerDir() {
	std::map<std::string, std::vector<std::string> >::const_iterator it;
    for (it = serverDir.begin(); it != serverDir.end(); ++it) {
        std::cout << "Key: " << it->first << std::endl;
        const std::vector<std::string>& values = it->second;
        for (std::vector<std::string>::const_iterator vit = values.begin(); vit != values.end(); ++vit) {
            std::cout << "  - " << *vit << std::endl;
        }
    }
}

void Server::printLocationDir() {
	std::map<std::string, std::map<std::string, std::vector<std::string> > >::const_iterator it;
    for (it = locationDir.begin(); it != locationDir.end(); ++it) {
        std::cout << "Outer Key: " << it->first << std::endl;
        const std::map<std::string, std::vector<std::string> >& innerMap = it->second;

        std::map<std::string, std::vector<std::string> >::const_iterator innerIt;
        for (innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt) {
            std::cout << "  Inner Key: " << innerIt->first << std::endl;
            const std::vector<std::string>& values = innerIt->second;

            for (std::vector<std::string>::const_iterator vit = values.begin(); vit != values.end(); ++vit) {
                std::cout << "    - " << *vit << std::endl;
            }
        }
    }
}

Server::Server(): _hints(), _server_socket(-1), _bind_address(NULL) {

	this->_hints.ai_family = AF_INET;      
	this->_hints.ai_socktype = SOCK_STREAM;
	this->_hints.ai_flags = AI_PASSIVE;

}


Server::~Server(){
    Logger::info("Treenode Destructor() Called!!!");
}