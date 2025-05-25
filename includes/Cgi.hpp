#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
class Client;

class Cgi{
public:
    //main function
    void execute(Client *c);
    
    //setup
    char **generateEnv(Client *c, std::string target);
    void reset();
    
    //getter
    char **getArgs();
    char **getEnv() const;
    int *getPipeIn();
    int *getPipeOut();
    int getCgiPid();
    size_t getBytesSended();
    int getCGIState();
    
    
    //setters
    void setArgs(std::string target, Client *c);
    void setEnv(char **envp);
    void setCGIState(int state);
    void incrementBytesSended(size_t bytes);

    Cgi();
    ~Cgi();
private:
    int cgi_state;
    char **args;
    char **envp;
    int pipe_in[2];
    int pipe_out[2];
    int cgi_pid;
    size_t bytes_sended;
    pid_t pid;
};

#endif // CGI_HPP