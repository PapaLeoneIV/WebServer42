#ifndef LOGGER
#define LOGGER

#include <iostream>
#include <ctime>
#include <sstream>
class Logger {
    private:
        Logger();
        static std::string currentTime();
    public:
        static void info(const std::string& message);
    
        static void error(const std::string& file, const std::string& message);
    };       
#endif //LOGGER