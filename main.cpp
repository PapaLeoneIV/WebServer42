#include "./includes/ServerManager.hpp"
#include "./includes/ConfigParser.hpp"
#include "./includes/Booter.hpp"
#include "./includes/Logger.hpp"

#include <csignal>




int main(int argc, char **argv)
{

    if (handle_arguments(argc, argv))
        return (1);

    ServerManager serverManager;
    ConfigParser configParser;
    configParser.init();
    std::vector<Server> servers = configParser.fromConfigFileToServers(argv[1]);
    if (configParser.validatePath(std::string(argv[1])) || servers.empty()){
        Logger::error(argv[1], "invalid configuration file");
        return 1;
    }
    Logger::info("Configuration file loaded successfully");
    std::vector<std::string> hostPortKeys;
    for(size_t i = 0; i < servers.size(); i++){
    
        Server server = servers[i];
        std::string host = server.getServerDir()["host"][0];
        std::string port = server.getServerDir()["listen"][0];
        server.setHostPortKey(host + ":" + port);
        bool shouldBoot = true;
        for(size_t i = 0; i < hostPortKeys.size(); i++){
            if(server.getHostPortKey() == hostPortKeys[i]){
                server.setServerSocket(servers[i].getServerSocket());
                shouldBoot = false;
            }
        }
        hostPortKeys.push_back(server.getHostPortKey());
        
        if(shouldBoot)
        Booter::bootServer(&server, host, port);
        std::string msg = "Server started on " + host + ":" + port;
        Logger::info(msg);
        serverManager.addServer(server);
    }
    Logger::info("Server/s started successfully");
    serverManager.eventLoop();
    std::cout << "Shutting down..." << std::endl;
    return 0;
}