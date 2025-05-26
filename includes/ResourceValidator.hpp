#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include <climits>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>



class Server;
class Client;
class Response;
class Request;

class ResourceValidator {
public:
    ResourceValidator();
    ~ResourceValidator();

    // HTTP Method Handlers
    static void handleGET(Client* c, std::string& path, bool isAutoIndexOn) ;
    static void handleDELETE(Client* c, const std::string& path);
    static void handleCGI(Client* c, const std::string& target,
                    std::map<std::string, std::vector<std::string> >& locationBlock) ;

    // Validation & Matching
    static void validateResource(Client* c, Server* s) ;
    static bool isValidCGIExtension(std::map<std::string, std::vector<std::string> >& locationBlock,
                             const std::string& urlFile);

    static mode_t checkResource(const std::string& path, Response* resp, int accessMode);
    static int deleteResource(const std::string& path, Response* resp, bool useDetailedResponse = true);
    static std::string getMatchingLocation(std::string url, Server* s);

    // Utilities
    static std::string readFile(const std::string& path, Response* resp);
    static std::string extractQueryParams(const std::string& url, const std::string& paramName,
                                   const std::string& defaultValue,
                                   const std::vector<std::string>& validValues);
    static bool isQueryParamValid(const std::string& url, const std::string& paramName, bool defaultValue = false);
    static std::string findBestApproximationString(const std::string& url, const std::vector<std::string>& dictionary);

    static unsigned int levenshteinDistance(const std::string& s1, const std::string& s2);
};

#endif // PARSER_HPP
