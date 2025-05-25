#include "../includes/Logger.hpp"
#include "../includes/ResourceValidator.hpp"
#include "../includes/Request.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Response.hpp"
#include "../includes/Cgi.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"

void ResourceValidator::validateResource(Client *c, Server *server)
{
    Request *req = c->getRequest();
    Response *resp = c->getResponse();
    
    Logger::info("Starting resource validation for c FD: " + wb_itos(c->getSocketFd()));
    // ─────────────────────────────────────────────────────────────
    // Enforce client_max_body_size limit (if defined)
    if(server->getServerDir().find("client_max_body_size") != server->getServerDir().end() 
        && (size_t)wb_stoi(server->getServerDir()["client_max_body_size"][0]) < req->getBody().size()) {
            resp->setStatusCode(413);
            resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
            return;    
    }
    Logger::info("Request URL: " + req->getUrl() + " [" + wb_itos(c->getSocketFd()) + "]");
    Logger::info("Searching for best matching location...");
    
    // ─────────────────────────────────────────────────────────────
    // Find best matching location
    std::string bestMatchingLocation = this->getMatchingLocation(c->getRequest()->getUrl(), c->getServer());
    if(bestMatchingLocation.empty()){
        resp->setStatusCode(404);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        return;
    }
    std::map<std::string, std::vector<std::string> > &locationBlock = server->getLocationDir()[bestMatchingLocation];
    Logger::info("Best matching location: " + bestMatchingLocation + " [" + wb_itos(c->getSocketFd()) + "]");
    
    // ─────────────────────────────────────────────────────────────
    //Check against Allowed Methods from config file
    bool foundMethod = locationBlock["allow_methods"].size() ? false  : true;
    for(size_t i = 0; i < locationBlock["allow_methods"].size(); i++){
        if(req->getMethod() == locationBlock["allow_methods"][i]){
            foundMethod = true;
        }
    }
    if(!foundMethod){
        Logger::info("HTTP method " + req->getMethod() + " not allowed for this location.");
        resp->setStatusCode(405);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        return;
    }
    // ─────────────────────────────────────────────────────────────
    //Choose between location root or server root
    bool isRootInLocationDirective = locationBlock["root"].size(); 
    std::string rootPath = isRootInLocationDirective  ? locationBlock["root"][0] : server->getServerDir()["root"][0];


    // ─────────────────────────────────────────────────────────────
    //Extract resource path and resource file
     std::string urlPath = req->getUrl().substr(0, req->getUrl().find_last_of("/")) + "/";
     std::string urlFile = req->getUrl().substr(req->getUrl().find_last_of("/") + 1, req->getUrl().size());

    
    // ─────────────────────────────────────────────────────────────
    //Check if is it a CGI Request
    Logger::info("Checking if CGI handling is required...");
    if(bestMatchingLocation.find("/cgi-bin") != std::string::npos){
        if(urlPath[urlPath.size() - 1] != '/' ){
            urlPath += "/";
        }
        Logger::info("Handling as CGI req: " + rootPath + urlPath + urlFile);
        return this->handleCGI(c, rootPath + urlPath + urlFile, locationBlock);
    }
    
    // ─────────────────────────────────────────────────────────────
    //Check for redirection
    if(locationBlock.find("return") != locationBlock.end()){
        Logger::info("Redirecting with return directive, code: " + locationBlock["return"][0]);
        resp->setStatusCode(wb_stoi(locationBlock["return"][0]));
        if(locationBlock["return"].size() > 1)
            resp->setHeaders("Location", locationBlock["return"][1]);
        return;
    }

    // ─────────────────────────────────────────────────────────────
    //Alias substitution
    if(locationBlock.find("alias") != locationBlock.end()){
        Logger::info("Applying alias: " + locationBlock["alias"][0]);
        urlPath = locationBlock["alias"][0]; 
    }
    
    // ─────────────────────────────────────────────────────────────
    //If 'root' is present i need to attach root to location path
    std::string filePath =  rootPath + urlPath + urlFile;
    Logger::info("Resolved file path: " + filePath + " [" + wb_itos(c->getSocketFd()) + "]");

    // ─────────────────────────────────────────────────────────────
    //Check if the user is requesting a directory 
    std::string indexFile = locationBlock.find("index") != locationBlock.end() ? locationBlock["index"][0] : server->getServerDir()["index"][0];
    bool isAutoIndexOn = locationBlock.find("autoindex") != locationBlock.end() ? locationBlock["autoindex"][0] == "on" : false; 
    
    // ─────────────────────────────────────────────────────────────
    //Attach index file to the path if the user is requesting a directory and autoindex is off
    if (filePath[filePath.size() - 1] == '/' && !isAutoIndexOn) {
        Logger::info("Directory requested. Appending index file: " + indexFile);
        filePath += indexFile;
    }

    // ─────────────────────────────────────────────────────────────
    //Handle GET, POST, DELETE 
    Logger::info("Handling HTTP method: " + req->getMethod());
    if (req->getMethod() == "GET") {
        this->handleGET(c, filePath, isAutoIndexOn);
    } else if (req->getMethod() == "POST") {
         resp->setStatusCode(204);
    } else if (req->getMethod() == "DELETE") {
        this->handleDELETE(c, filePath);
    } else {
        resp->setStatusCode(501);
        resp->setBody(getErrorPage(501, server));
        return;
    }
}

#define URL_MATCHING_ERROR_ALLOWANCE 1

unsigned int ResourceValidator::levenshteinDistance(const std::string& s1, const std::string& s2)
{
	const std::size_t len1 = s1.size(), len2 = s2.size();
	std::vector<std::vector<unsigned int> > d(len1 + 1, std::vector<unsigned int>(len2 + 1));

	d[0][0] = 0;
	for(unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
	for(unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

	for(unsigned int i = 1; i <= len1; ++i)
		for(unsigned int j = 1; j <= len2; ++j)
                      d[i][j] = std::min(std::min(  d[i - 1][j] + 1,
                                                    d[i][j - 1] + 1),
                                                    d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));  
	return d[len1][len2];
}

std::string ResourceValidator::findBestApproximationString(const std::string& url, const std::vector<std::string>& dictionary) {

    int min_distance = INT_MAX;
    std::string best_match = "";
    size_t i;

    for (i = 0; i < dictionary.size(); i++) {
        int distance = levenshteinDistance(url, dictionary[i]);
        if (distance < URL_MATCHING_ERROR_ALLOWANCE && distance < min_distance) {
            min_distance = distance;
            best_match = dictionary[i];
        }
    }

    Logger::info("Best matching found: '" + best_match + "'");
    return best_match;
}

std::string ResourceValidator::getMatchingLocation(std::string url, Server* server) {
    std::vector<std::string> dictionary;

    std::map<std::string, std::map<std::string, std::vector<std::string> > >::iterator it = server->getLocationDir().begin();
    for (; it != server->getLocationDir().end(); ++it) {
        dictionary.push_back(it->first);
    }

    size_t pos = url.find('?');
    if (pos != std::string::npos)
        url = url.substr(0, pos);

    if (!url.empty() && url[url.size() - 1] != '/') {
        size_t lastSlash = url.find_last_of('/');
        if (lastSlash != std::string::npos) {
            std::string lastPart = url.substr(lastSlash + 1);

            if (lastPart.find('.') != std::string::npos) {
                url = url.substr(0, lastSlash + 1);
                if (url.empty())
                    url = "/";
            }
        }
    }
    Logger::info("The URL used for matching the location block: " + url);
    return findBestApproximationString(url, dictionary);
}



int ResourceValidator::deleteResource(const std::string& filePath, Response* resp, bool useDetailedResponse) {
    if (access(filePath.c_str(), F_OK) != 0) {
        resp->setStatusCode(404);
        return FAILURE;
    }
    
    struct stat fileInfo;
    std::string fileDetails = "";
    if (useDetailedResponse && stat(filePath.c_str(), &fileInfo) == 0) {
        fileDetails = "File: " + filePath + "\n";
        fileDetails += "Size: " + wb_itos(fileInfo.st_size) + " bytes\n";
        fileDetails += "Last modified: " + std::string(ctime(&fileInfo.st_mtime));
        fileDetails += "Content: " + this->readFile(filePath, resp);
    }
    
    if (stat(filePath.c_str(), &fileInfo) == 0 && S_ISDIR(fileInfo.st_mode)) {
        resp->setStatusCode(403);
        Logger::error("ResourceValidator", "Cannot delete directory: " + filePath);
        return FAILURE;
    }

    int fileType = this->checkResource(filePath, resp, W_OK);
    if (fileType == FAILURE) {
        Logger::error("ResourceValidator", "Resource check failed: " + filePath + " (Status: " + wb_itos(resp->getStatus()) + ")");
        return FAILURE;
    }


    // Secondo HTTP 1.1:
    // - 204 No Content: quando non c'è bisogno di inviare un corpo nella risposta
    // - 200 OK: quando si vuole inviare un corpo nella risposta (es. conferma, statistiche, ecc.)
    if (useDetailedResponse) {
        resp->setStatusCode(200);
        std::string successBody = "<html><body>\n<h1>DELETE req processed successfully</h1>\n";
        successBody += "<p>The server has accepted the DELETE req for the following resource:</p>\n";
        successBody += "<pre>\n" + fileDetails + "</pre>\n";
        successBody += "<p>Note: Resources are not actually deleted as per server configuration.</p>\n";
        successBody += "</body></html>\n";
        resp->setBody(successBody);
    } else {
        resp->setStatusCode(204);
    }

    if (std::remove(filePath.c_str()) != 0) {
        Logger::error(__FILE__, "Error deleting file");
        resp->setStatusCode(500);
        return FAILURE;
    }

    return SUCCESS;
}


void ResourceValidator::handleDELETE(Client* c, const std::string& filePath) {
    Request *req = c->getRequest();
    Response *resp = c->getResponse();


    Logger::info("DELETE req for: " + filePath + " [" + wb_itos(c->getSocketFd()) + "]");
        
    bool useDetailedResponse = isQueryParamValid(req->getUrl(), "details", false);
    int result = this->deleteResource(filePath, resp, useDetailedResponse);
    if (result == SUCCESS) {
        if (resp->getStatus() == 204) {
            Logger::info("DELETE req processed successfully (204 No Content): " + filePath + " [" + wb_itos(c->getSocketFd()) + "]");
        } else {
            Logger::info("DELETE req processed successfully (200 OK): " + filePath + " [" + wb_itos(c->getSocketFd()) + "]");
        }
    } else {
        Logger::error("ResourceValidator", "Failed to process DELETE req: " + filePath + " (Status: " + wb_itos(resp->getStatus()) + ") [" + wb_itos(c->getSocketFd()) + "]");

        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
    }
    return;
}


void ResourceValidator::handleGET(Client* c, std::string& filePath, bool isAutoIndexOn){

    Request *req = c->getRequest();
    Response *resp = c->getResponse();

    int fileType;
    //checcko se la risorsa richiesta è accessibile
    if(!(fileType = this->checkResource(filePath, resp, W_OK))){
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        return;
    }
    
    if(S_ISDIR(fileType) && isAutoIndexOn){
    
        if(filePath[filePath.size() - 1] != '/'){
            std::string newUrl = req->getUrl() + "/";
            filePath += "/";
        }
        std::string dirBody = fromDIRtoHTML(filePath, req->getUrl());
        if(dirBody.empty()){
            resp->setStatusCode(500);
            return;
        }

        resp->setBody(dirBody);
        return;
    
    } else if(S_ISDIR(fileType) && !isAutoIndexOn){
        resp->setStatusCode(403);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        return;  
    }
    
    if(resp->getStatus() != 200){
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        return;
    }
    
    Logger::info("Reading file: " + filePath + " [" + wb_itos(c->getSocketFd()) + "]");
    resp->setBody(this->readFile(filePath, resp));

    return ;
}


bool ResourceValidator::isValidCGIExtension( std::map<std::string, std::vector<std::string> >& locationBlock, const std::string& urlFile){
    std::vector<std::string> &cgiExtensions = locationBlock["cgi_ext"];
    unsigned long isdotpresent = urlFile.find_last_of(".");
    if(isdotpresent == std::string::npos)
        return false;
    std::string extension = urlFile.substr(isdotpresent, urlFile.size());
    for(size_t i = 0; i < cgiExtensions.size(); i++){
        if(extension == cgiExtensions[i]){
            return true;
        }
    }
    Logger::error("ResourceValidator", "Invalid CGI extension: " + extension);
    for(size_t i = 0; i < cgiExtensions.size(); i++){
        Logger::error("ResourceValidator", cgiExtensions[i]);
    }
    return false;
}


ResourceValidator::ResourceValidator() {}

ResourceValidator::~ResourceValidator(){}



void ResourceValidator::handleCGI(Client* c, const std::string& target,
                std::map<std::string, std::vector<std::string> >& locationBlock){
    
    Request *req = c->getRequest();
    Response *resp = c->getResponse();
    if(!req || !resp )
        return;

    if(target.empty()){
        resp->setStatusCode(404);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        req->setState(StateParsingError);
        return;
    }
    //devo contrallare che il file esista e sia eseguibile
    if (access(target.c_str(), F_OK) != 0) {
        resp->setStatusCode(404);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        req->setState(StateParsingError);
        return;
    }
    if (access(target.c_str(), X_OK) != 0) {
        resp->setStatusCode(403);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        req->setState(StateParsingError);
        return;
    }
    //check if the file is a cgi
    if(!this->isValidCGIExtension(locationBlock, target)){
        resp->setStatusCode(403);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        req->setState(StateParsingError);
        return;
    }

    char **envp = c->getCgiObj().generateEnv(c, target);
    if(envp == NULL){
        resp->setStatusCode(500);
        resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
        req->setState(StateParsingError);
        return;
    }
    Logger::info("CGI ResourceValidator: Completed parsing: Init Env");
    c->getCgiObj().setEnv(envp);
    c->getCgiObj().setArgs(target, c);
    c->getCgiObj().execute(c);
    c->getCgiObj().setCGIState(1);

    return ;
}