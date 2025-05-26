#include "../includes/Logger.hpp"

std::string Logger::currentTime() {
    char buffer[20];
    std::time_t now = std::time(NULL);
    std::tm* tm_info = std::localtime(&now);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buffer);
}


void Logger::info(const std::string& message) {
         std::cout << "\033[1;" << 32 << "m[" << "INFO" << "]\033[0m "
              << "[" << currentTime() << "] "
              << message << std::endl;
}

void Logger::error(const std::string& file, const std::string& message) {
        std::cout << "\033[1;31m[ERROR]\033[0m \033[1m" << file << "\033[0m " << message << std::endl;
}

Logger::Logger(){};
