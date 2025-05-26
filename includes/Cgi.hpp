#ifndef CGI_HPP
#define CGI_HPP

#include <vector>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

class Client;

class Cgi{
public:
    //main function
    void execute(const Client *c);
    
    //setup
    static char **generateEnv(const Client *c, const std::string& target);
    void reset();
    
    //getter
    char **getArgs() const;
    char **getEnv() const;
    int *getPipeIn();
    int *getPipeOut();
    int getCgiPid() const;
    size_t getBytesSended() const;
    int getCGIState() const;
    
    
    //setters
    void setArgs(const std::string& target, const Client *c);
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