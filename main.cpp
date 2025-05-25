#include "./includes/ServerManager.hpp"
#include "./includes/ConfigParser.hpp"
#include "./includes/Booter.hpp"
#include "./includes/Logger.hpp"


int main(int argc, char **argv)
{
    if (handle_arguments(argc, argv))
        return (1);

    ConfigParser cp;
    cp.init();

    bool isConfigPathInvalid = cp.validatePath(std::string(argv[1]));
    if (isConfigPathInvalid){
        Logger::error(argv[1], "Invalid configuration file path");
        return 1;
    }

    std::vector<Server> servers = cp.extractServerConfiguration(argv[1]);
    if (servers.empty()){
        Logger::error(argv[1], "Invalid configuration file");
        return 1;
    }

    ServerManager sm;
    Booter::boot(servers, sm);
    Logger::info("Server/s started successfully");

    sm.eventLoop();

    Logger::info("Shutting down...");
    return 0;
}