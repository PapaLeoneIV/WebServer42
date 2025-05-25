#include "../includes/Logger.hpp"


void Logger::info(const std::string& message) {
        std::cout << "\033[1;32m[INFO]\033[0m : " << message  << std::endl;
}

void Logger::error(const std::string& file, const std::string& message) {
        std::cout << "\033[1;31m[ERROR]\033[0m \033[1m" << file << "\033[0m " << message << std::endl;
}

Logger::Logger(){};
