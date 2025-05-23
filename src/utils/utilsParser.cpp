#include "../../includes/Parser.hpp"
#include "../../includes/Response.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Utils.hpp"

int Parser::checkResource(std::string filePath, Response* response, int accessMode) {
    
    struct stat sb;
    if (access(filePath.c_str(), F_OK) == -1) {
        response->setStatusCode(404);
        return -1;
    }

    if (stat(filePath.c_str(), &sb) == -1) {
        response->setStatusCode(500);
        return -1;
    }

    if (S_ISREG(sb.st_mode) || S_ISDIR(sb.st_mode)) {
        if (checkPermissions(filePath, accessMode) != SUCCESS) {
            response->setStatusCode(403);
            return -1;
        }
    } else {
        response->setStatusCode(403);
        return -1;
    }

    return sb.st_mode;
}


std::string Parser::readFile(std::string filePath, Response *response)
{
    std::string fileContent;
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
        response->setStatusCode(404);
        return "";
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    for (std::vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it) {
        fileContent.push_back(*it);
    }

    return fileContent;
}

std::string Parser::extractQueryParams(const std::string &url, const std::string  &paramName, const std::string &defaultValue, const std::vector<std::string> &validValues) {
    size_t queryPos = url.find('?');
    if (queryPos == std::string::npos) {
        return defaultValue;
    }
    
    std::string queryString = url.substr(queryPos + 1);
    std::string paramPrefix = paramName + "=";
    
    size_t paramPos = queryString.find(paramPrefix);
    if (paramPos == std::string::npos) {
        return defaultValue;
    }
    
    size_t valueStart = paramPos + paramPrefix.length();
    size_t valueEnd = queryString.find('&', valueStart);
    if (valueEnd == std::string::npos) {
        valueEnd = queryString.length();
    }
    
    std::string value = queryString.substr(valueStart, valueEnd - valueStart);

    if (validValues.empty()) {
        return value;
    }

    for (size_t i = 0; i < validValues.size(); i++) {
        if (value == validValues[i]) {
            return value;
        }
    }
    
    return defaultValue;
}

bool Parser::isQueryParamValid(const std::string& url, const std::string& paramName, bool defaultValue) {
    std::vector<std::string> trueValues;
    trueValues.push_back("true");
    trueValues.push_back("1");
    trueValues.push_back("TRUE");

    std::vector<std::string> falseValues;
    falseValues.push_back("false");
    falseValues.push_back("0");
    falseValues.push_back("FALSE");
    
    std::string value = extractQueryParams(url, paramName, defaultValue ? "true" : "false", trueValues);

    if (value.empty()) {
        return defaultValue;
    }

    for (size_t i = 0; i < trueValues.size(); i++) {
        if (value == trueValues[i]) {
            return true;
        }
    }

    for (size_t i = 0; i < falseValues.size(); i++) {
        if (value == falseValues[i]) {
            return false;
        }
    }
    
    return defaultValue;
}
