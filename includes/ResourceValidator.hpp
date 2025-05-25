#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unistd.h>  // for access()

class Server;
class Client;
class Response;
class Request;

class ResourceValidator {
public:
    ResourceValidator();
    ~ResourceValidator();

    // HTTP Method Handlers
    void handleGET(Client* c, std::string& filePath, bool isAutoIndexOn);
    void handleDELETE(Client* c, const std::string& filePath);
    void handleCGI(Client* c, const std::string& target,
                    std::map<std::string, std::vector<std::string> >& locationBlock);

    // Validation & Matching
    void validateResource(Client* c, Server* server);
    bool isValidCGIExtension(std::map<std::string, std::vector<std::string> >& locationBlock,
                             const std::string& urlFile);
    int checkResource(const std::string& filePath, Response* resp, int accessMode);
    int deleteResource(const std::string& filePath, Response* resp, bool useDetailedResponse = true);
    std::string getMatchingLocation(std::string url, Server* server);

    // Utilities
    std::string readFile(const std::string& filePath, Response* resp);
    std::string extractQueryParams(const std::string& url, const std::string& paramName,
                                   const std::string& defaultValue,
                                   const std::vector<std::string>& validValues);
    bool isQueryParamValid(const std::string& url, const std::string& paramName, bool defaultValue = false);
    std::string findBestApproximationString(const std::string& url, const std::vector<std::string>& dictionary);
    unsigned int levenshteinDistance(const std::string& s1, const std::string& s2);
};

#endif // PARSER_HPP
