#include "../includes/Logger.hpp"
#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"

//GETTERS
std::string &Server::gethostPortPair() {return this->hostPortPair;};
SOCKET      Server::getServerSocket() const	{return this->serverSocket;}
fd_set      Server::getFdsSet() const {return this->fds;}
addrinfo    &Server::getHints()	{return this->hints;}
addrinfo    *Server::getBindAddress() const {return this->bindAddress;}
std::map<std::string, std::vector<std::string> > &Server::getServerDir() {return this->serverDir;}
std::map<std::string, std::map<std::string, std::vector<std::string> > > &Server::getLocationDir() {return this->locationDir;}


//SETTERS
void    Server::sethostPortPair(const std::string& hostPortPair){this->hostPortPair = hostPortPair;};
void    Server::setServerSocket(SOCKET server_socket)   {this->serverSocket = server_socket;};
void    Server::setFds(const fd_set &fds)	{this->fds = fds;};
void    Server::setHints(const addrinfo &hints) {this->hints = hints;};
void    Server::setBindAddress(addrinfo *bind_address)  {this->bindAddress = bind_address;};
void	Server::setLocationDir(const std::string& locationPath, const std::string& key,  const std::vector<std::string>& value) {this->locationDir[locationPath][key] = value;}

void	Server::setServerDir(std::string key, std::vector<std::string> value) {
	                                            //it's done to handle multiple error pages directives from the config fil
    if(key == "error_page" && value.size() > 1) //Todo: find a better solution 
	{
		key.append("_");
		key.append(value[0]);
		value.erase(value.begin());
	}
	this->serverDir[key] = value;
}


Server::Server(): hints(), serverSocket(-1), fds(), bindAddress(NULL) {

	this->hints.ai_family = AF_INET;      
	this->hints.ai_socktype = SOCK_STREAM;
	this->hints.ai_flags = AI_PASSIVE;

}


Server::~Server(){}