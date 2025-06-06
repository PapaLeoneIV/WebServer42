#include "../../includes/ResourceValidator.hpp"
#include "../../includes/Response.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Utils.hpp"

mode_t ResourceValidator::checkResource(const std::string& path, Response* resp, int accessMode) {
    
    struct stat sb = {};
    if (access(path.c_str(), F_OK) == -1) {
        resp->setStatusCode(404);
        return 0;
    }

    if (stat(path.c_str(), &sb) == -1) {
        resp->setStatusCode(500);
        return 0;
    }

    if (S_ISREG(sb.st_mode) || S_ISDIR(sb.st_mode)) {
        if (access(path.c_str(), accessMode)) {
            resp->setStatusCode(403);
            return 0;
        }
    } else {
        resp->setStatusCode(403);
        return 0;
    }

    return sb.st_mode;
}


std::string ResourceValidator::readFile(const std::string& path, Response* resp)
{
    std::string fileContent;
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Error: Unable to open file " << path << std::endl;
        resp->setStatusCode(404);
        return "";
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    for (std::vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it) {
        fileContent.push_back(*it);
    }

    return fileContent;
}

std::string ResourceValidator::extractQueryParams(const std::string &url, const std::string  &paramName, const std::string &defaultValue, const std::vector<std::string> &validValues) {
    const size_t queryPos = url.find('?');
    if (queryPos == std::string::npos) {
        return defaultValue;
    }
    
    std::string queryString = url.substr(queryPos + 1);
    const std::string paramPrefix = paramName + "=";

    const size_t paramPos = queryString.find(paramPrefix);
    if (paramPos == std::string::npos) {
        return defaultValue;
    }

    const size_t valueStart = paramPos + paramPrefix.length();
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

bool ResourceValidator::isQueryParamValid(const std::string& url, const std::string& paramName, bool defaultValue) {
    std::vector<std::string> trueValues;
    trueValues.push_back("true");
    trueValues.push_back("1");
    trueValues.push_back("TRUE");

    std::vector<std::string> falseValues;
    falseValues.push_back("false");
    falseValues.push_back("0");
    falseValues.push_back("FALSE");

    const std::string value = extractQueryParams(url, paramName, defaultValue ? "true" : "false", trueValues);

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
