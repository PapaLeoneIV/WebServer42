#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <limits.h>

#include <map>
#include <stack>
#include <string>
#include <vector>

#include "../includes/Server.hpp"
#include "../includes/ServerManager.hpp"
#include "../includes/Treenode.hpp"


typedef int (*FunctionPtr)(std::vector<std::string>);

typedef std::map<std::string, std::vector<std::string> > ConfigDirectiveMap;

typedef std::vector<std::string> directiveValueVector;

class ConfigParser
{
public:
   std::vector<Server> fromConfigFileToServers(char *file);
  int validatePath(std::string path);
  Treenode *createConfigTree(std::string path);
  int isValidDirective(std::string token);
  bool verifyDirectives(Server &server);
  int checkMandatoryDirectives(Server &server);
  int extractDirectives(Server &server, Treenode *node);
  int checkForAllowdMultipleDirectives(std::string directive);
  
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

  void setFileName(std::string path);
  std::string getFileName();
  void init();
void deleteTree(Treenode *node);

  ConfigParser();
  ~ConfigParser();

  std::map<std::string, int (ConfigParser::*)(std::vector<std::string>)> fnToParseDirectives;
  std::vector<std::string> directives;
  std::vector<std::string> extensionsAllowd;

private:
  std::string fileName;
};

std::string removeComments(std::ifstream &file);
std::string trimLeftRight(const std::string &str);
std::string removeEmptyLines(const std::string &input);
std::string trimm(const std::string &input);
int         setUpDefaultDirectiveValues(Server *server);

#endif // CONFIGPARSER