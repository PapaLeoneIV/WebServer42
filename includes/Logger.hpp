#ifndef LOGGER
#define LOGGER

#include <iostream>


class Logger {
    private:
        Logger();
    
    public:
        static void info(const std::string& message);
    
        static void error(const std::string& file, const std::string& message);
    };



        
#endif //LOGGER