#ifndef PARSER_HPP
#define PARSER_HPP

#include <set>
#include <string>
#include <sys/unistd.h>
#include <vector>
#include <vector>
#include <map>
#include <sys/types.h>
#include <vector>

class Server;
class Client;
class Response;

class Request;

class Parser{
    public:
    void handleGET(Client *client, std::string filePath, bool isAutoIndexOn);
    void handleDELETE(Client *client, std::string filePath);
    void handleCGI(Client *client, std::string target,  std::map<std::string, std::vector<std::string> > locationConfig);
    bool isValidCGIExtension(std::map<std::string, std::vector<std::string> > &locationConfig, std::string urlFile);
    void validateResource(Client *client, Server *server);
    std::string readFile(std::string filePath, Response *response);
    int checkResource(std::string filePath, Response *response, int accessMode = R_OK);
    int deleteResource(std::string filePath, Response *response, bool useDetailedResponse = true);
    std::string extractQueryParams(const std::string &url, const std::string  &paramName, const std::string &defaultValue="", const std::vector<std::string> &validValues = std::vector<std::string>());
    bool    isQueryParamValid(const std::string &url, const std::string &paramName, bool defaultValue = false);
    std::string  findBestApproximationString(std::string url, std::vector<std::string> dictionary);
    unsigned int levenshteinDistance(const std::string& s1, const std::string& s2);
	std::string getMatchingLocation(std::string url, Server *server);
    Parser();
    ~Parser();
};

#endif
