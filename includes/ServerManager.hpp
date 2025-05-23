#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <string>
#include <map>
#include "../includes/Utils.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

typedef int SOCKET;
class ServerManager{

	public:

	ServerManager();
	~ServerManager();

	void	eventLoop();
	
	void	registerNewConnections(SOCKET serverFd, Server *server);
	void	processRequest(Client *client);
	void	sendResponse(SOCKET fd, Client *client);
	void 	sendCgiBody(SOCKET fd, Client *client, Cgi &cgi_obj);
	void	readCgiBody(SOCKET fd, Client *client, Cgi &cgi_obj);
	void	initFdSets();
	void 	assignServer(Client *client);
	void	closeClientConnection(SOCKET fd, Client* client);
	const std::string getClientIP(Client *client);
	void	removeClient(SOCKET fd);
	void	addServer(Server &server);
	void	addToSet(SOCKET fd, fd_set *fdSet);
	void	removeFromSet(SOCKET fd, fd_set *fd_set);
	void	handleClientTimeout(time_t currentTime);
	
	std::vector<Server>	getServerMap();
	Client	*getClient(SOCKET clientFd);
	
	private:

	struct timeval timeout;
	
	fd_set	_masterPool;
	fd_set	_readPool;
	fd_set	_writePool;

	int		_maxSocket;

	std::map<SOCKET, Client*>	_clients_map;
	std::vector<Server>	_servers_map;
};


#endif
