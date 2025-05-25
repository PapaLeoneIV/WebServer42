#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <map>
#include <stack>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>


#include "../includes/Server.hpp"
#include "../includes/ServerManager.hpp"
#include "../includes/Treenode.hpp"

// Type aliases for readability
typedef int (*FunctionPtr)(std::vector<std::string>);

typedef std::map<std::string, std::vector<std::string> > ConfigDirectiveMap;

typedef std::vector<std::string> directiveValueVector;

class ConfigParser {
public:
    ConfigParser();
    ~ConfigParser();

    // Main entry point
    std::vector<Server> extractServerConfiguration(char* file);

    //Bootstrap
    void init();

    // File utilities
    void setFileName(const std::string& path);
    std::string getFileName();
    int validatePath(const std::string& path);
    void deleteTree(Treenode* node);

    // Tree & Directive processing
    Treenode* createConfigTree(const std::string& path);
    int extractDirectives(Server& server, Treenode* node);
    int isValidDirective(const std::string& token);
    bool verifyDirectives(Server& server);
    int checkMandatoryDirectives(Server& server);
    int checkForAllowdMultipleDirectives(const std::string& directive);

    // Directive parsing
    int parseListenValues(directiveValueVector v);
    int parseHostValues(directiveValueVector v);
    int parseServerNameValues(directiveValueVector v);
    int parseErrorPageValues(directiveValueVector v);
    int parseClientMaxBodyValues(directiveValueVector v);
    int parseRootValues(directiveValueVector v);
    int parseIndexValues(directiveValueVector v);
    int parseAutoIndexValues(directiveValueVector v);
    int parseAllowMethodsValues(directiveValueVector v);
    int parseReturnValues(directiveValueVector v);
    int parseAliasValues(directiveValueVector v);
    int parseCgiExtValues(directiveValueVector v);
    int parseCGIPathValues(directiveValueVector v);
    int parseProxyPassValues(directiveValueVector v);

    // Data members
    std::map<std::string, int (ConfigParser::*)(directiveValueVector)> parsingFunctions;
    std::vector<std::string> directives;
    std::vector<std::string> extensionsAllowd;

private:
    std::string fileName;
};

// Utility functions
std::string removeComments(std::ifstream& file);
std::string trimLeftRight(const std::string& str);
std::string removeEmptyLines(const std::string& input);
std::string trimm(const std::string& input);
int setUpDefaultDirectiveValues(Server* server);

#endif // CONFIGPARSER_HPP
