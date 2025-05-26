#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"
#include "../includes/ConfigParser.hpp"

int handle_arguments(int argc, char **argv)
{
    // help command
    if (argc == 2){
        if (std::string(argv[1]) == "--help"){
            Logger::info("Usage: webserver { [OPTIONS] || <config-filepath> }");
            Logger::info("         --help                           Display this help and exit");
            Logger::info("         -t <config-filepath>             Test the configuration file");
            Logger::info("         -v                               Display the current version");
            return (1);
        }
        // version command
        if (std::string(argv[1]) == "-v"){
            Logger::info("webserver version: webserver/" VERSION);
            return (1);
        }
        // testing config file command
        if (std::string(argv[1]) == "-t"){
            Logger::info("webserver config-file testing: missing <config-filepath>");
            return (1);
        }

        return 0;
    }
    if (argc == 3){
        if (std::string(argv[1]) != "-t"){
            Logger::info("webserver: try 'webserver --help' for more information");
            return (1);
        }
        ConfigParser configParser;
        configParser.init();

        bool isConfigPathInvalid = configParser.validatePath(std::string(argv[2]));
        if (isConfigPathInvalid){
            Logger::error(argv[1], "Invalid configuration file path");
            return 1;
        }

        std::vector<Server> servers = configParser.extractServerConfiguration(argv[2]);
        if (servers.empty()){
            Logger::error(argv[1], "Invalid configuration file");
            return 1;
        }
        Logger::info("webserver config-file testing: OK");
        return (1);
    }
    Logger::info("webserver: try 'webserver --help' for more information");
    return (1);
}

std::string fromDIRtoHTML(const std::string& path, const std::string& url)
{
    std::string html;
    html += "<!DOCTYPE html>";
    html += "<html lang=\"en\">";
    html += "<head>";
    html += "<meta charset=\"UTF-8\">";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<title>WebServer</title>";
    html += "</head>";
    html += "<body><ul>";

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.c_str())) != NULL){
        while ((ent = readdir(dir)) != NULL){
            std::string name = ent->d_name;

            if (name == "." || name == "..")
                continue;

            std::string fullPath = path + "/" + name;
            struct stat st = {};
            if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)){
                name += "/";
            }

            std::string href = (url != "/" ? url + name : name);
            html += "<li><a href=\"" + href + "\">" + name + "</a></li>";
        }
        html += "</ul></body></html>";
        closedir(dir);
    }else{
        return "";
    }

    return html;
}

std::string ErrToStr(const int error)
{
    switch (error)
    {
    case SUCCESS:
        return "Success";
    case ERR_RESOLVE_ADDR:
        return "Could not resolve address";
    case ERR_SOCK_CREATION:
        return "Error: socket creation failed";
    case ERR_SOCKET_NBLOCK:
        return "Error: setting socket to non-blocking failed";
    case ERR_BIND:
        return "Error: bind failed";
    case ERR_LISTEN:
        return "Error: listen failed";
    case ERR_SELECT:
        return "Error: select failed";
    case ERR_SEND:
        return "Error: send failed";
    case ERR_RECV:
        return "Error: recv failed: closing connection";
    case INVALID_METHOD:
        return "Error: the method is not supported (yet)";
    case INVALID_URL:
        return "Error: Invalid URL";
    case INVALID_VERSION:
        return "Error: HTTP version not supported";
    case INVALID_MANDATORY_HEADER:
        return "Error: Missing mandatory header";
    case INVALID_BODY:
        return "Error: Invalid body";
    case INVALID_BODY_LENGTH:
        return "Error: Invalid body length";
    case INVALID_MAX_REQUEST_SIZE:
        return "Error: Request too long";
    case INVALID_CONNECTION_CLOSE_BY_CLIENT:
        return "Error: Connection closed by c";
    case INVALID_REQUEST:
        return "Error: Invalid req";
    case INVALID_CONTENT_LENGTH:
        return "Error: Invalid content length";
    case ERR_FCNTL:
        return "Error: fcntl failed";
    case FILE_NOT_FOUND:
        return "Error: File not found";
    case FILE_READ_DENIED:
        return "Error: Read access denied";
    default:
        return "Unknown Error";
    }
}

std::string getContentType(const std::string &url, const int status)
{
    if (status != 200)
        return "text/html";
    if (url == "/")
        return "text/html";
    if (*(url.rbegin()) == '/')
        return "text/html";
    const size_t idx = url.find_last_of('.');
    if (idx == std::string::npos)
        return "text/plain";
    const std::string extension = url.substr(idx);
    if (extension.empty())
        return "text/plain";
    const std::string urlC = &extension[0];
    if (urlC == ".css")
        return "text/css";
    if (urlC == ".csv")
        return "text/csv";
    if (urlC == ".gif")
        return "image/gif";
    if (urlC == ".htm")
        return "text/html";
    if (urlC == ".html")
        return "text/html";
    if (urlC == ".ico")
        return "image/x-icon";
    if (urlC == ".jpeg")
        return "image/jpeg";
    if (urlC == ".jpg")
        return "image/jpeg";
    if (urlC == ".js")
        return "application/javascript";
    if (urlC == ".json")
        return "application/json";
    if (urlC == ".png")
        return "image/png";
    if (urlC == ".pdf")
        return "application/pdf";
    if (urlC == ".svg")
        return "image/svg+xml";
    if (urlC == ".txt")
        return "text/plain";
    return "text/plain";
}

std::string fromCodeToMsg(const int status)
{
    switch (status)
    {
    case 200:
        return "OK";
    case 204:
        return "No Content";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 307:
        return "Temporary Redirect";
    case 400:
        return "Bad Request";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 411:
        return "Length Required";
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 505:
        return "HTTP Version Not Supported";
    default:
        return "Status Code not recognized";
    }
}

std::string getErrorPage(const int status, Server *s)
{
    Logger::info("Trying to get error page for status: " + wb_itos(status));

    if (s != NULL){
        for (std::map<std::string, std::vector<std::string> >::iterator it = s->getServerDir().begin(); it != s->getServerDir().end(); ++it){
            if (it->first == ("error_page_" + wb_itos(status)) && !it->second.empty()){
                Logger::info("Found error_page directive for status: " + wb_itos(status) + ", path: " + it->second[0]);
                return readTextFile(it->second[0]);
            }
        }
        Logger::info("No error_page directive found for status: " + wb_itos(status) + ", using default");
    }else{Logger::info("Server is NULL, using default error page");}
    std::string errorPath;
    switch (status)
    {
    case 400:
        errorPath = "./static/default-error-page/400.html";
        break;
    case 403:
        errorPath = "./static/default-error-page/403.html";
        break;
    case 404:
        errorPath = "./static/default-error-page/404.html";
        break;
    case 405:
        errorPath = "./static/default-error-page/405.html";
        break;
    case 411:
        errorPath = "./static/default-error-page/411.html";
        break;
    case 413:
        errorPath = "./static/default-error-page/413.html";
        break;
    case 414:
        errorPath = "./static/default-error-page/414.html";
        break;
    case 500:
        errorPath = "./static/default-error-page/500.html";
        break;
    case 501:
        errorPath = "./static/default-error-page/501.html";
        break;
    case 505:
        errorPath = "./static/default-error-page/505.html";
        break;
    default:
        Logger::error("Utils", "No default error page for status: " + wb_itos(status));
        return "<html><body><h1>Errore " + wb_itos(status) + "</h1><p>" + fromCodeToMsg(status) + "</p></body></html>";
    }

    Logger::info("Using default error page: " + errorPath);
    return readTextFile(errorPath);
}

int wb_stoi(const std::string& str)
{
    std::stringstream ss(str);
    int number;
    ss >> number;
    return number;
}

int wb_stox(const std::string &str)
{
    int number = 0;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> number;
    return number;
}

std::string wb_itos(const int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string wb_ltos(const long number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string printFd(int fd){return " [" + wb_itos(fd) + "]";}

std::string to_lower(const std::string &input)
{
    std::string result = input;
    for (size_t i = 0; i < result.size(); i++){
        result[i] = std::tolower(static_cast<unsigned char>(result[i]));
    }
    return result;
}

std::string readTextFile(const std::string& path)
{
    std::string fileContent;
    std::ifstream file(path.c_str(), std::ios::in);
    std::string line;
    while (std::getline(file, line)){
        fileContent += line + "\n";
    }
    file.close();
    return fileContent;
}