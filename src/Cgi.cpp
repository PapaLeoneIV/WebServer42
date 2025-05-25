#include "../includes/Cgi.hpp"
#include "../includes/Client.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"


void Cgi::setArgs(std::string target, Client *c) {
  Response *resp = c->getResponse();

  this->args = static_cast<char **>(calloc(3, sizeof(char *)));
  if (!args) {
    Logger::error(__FILE__, "Allocating memory for args failed");
    resp->setStatusCode(500);
    resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
    return;
  }
  this->args[0] = strdup(target.c_str());
  if (!args[0]) {
    free(args);
    Logger::error(__FILE__, "Allocating memory for args[0] failed");
    resp->setStatusCode(500);
    resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
    return;
  }
  this->args[1] = strdup(target.c_str());
  if (!args[1]) {
    free(args[0]);
    free(args);
    Logger::error(__FILE__, "Allocating memory for args[1] failed");
    resp->setStatusCode(500);
    resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
    return;
  }
  this->args[2] = NULL;
}

// https://datatracker.ietf.org/doc/html/rfc3875#section-4.1
char **Cgi::generateEnv(Client *c, std::string target) {
  Request *req = c->getRequest();
  Response *resp = c->getResponse();
  if (!req || !resp)
    return NULL;

  std::vector<std::string> env_vec;

  env_vec.push_back("AUTH_TYPE=Basic");
  env_vec.push_back("GATEWAY_INTERFACE=CGI/1.1");
  env_vec.push_back("PATH_INFO=" + target);
  env_vec.push_back("PATH_TRANSLATED=" + target);
  env_vec.push_back("REQUEST_URI=" + target);
  env_vec.push_back("SERVER_SOFTWARE=URMOM");
  env_vec.push_back("QUERY_STRING=" + req->getQueryParam());
  env_vec.push_back("REQUEST_METHOD=" + req->getMethod());
  env_vec.push_back("SCRIPT_FILENAME=" + target);
  env_vec.push_back("SCRIPT_NAME=" + target);
  env_vec.push_back("SERVER_PROTOCOL=HTTP/1.1");

  if (req->getMethod() == "POST") {
    std::map<std::string, std::string> &headers = req->getHeaders();
    std::string cl = headers.count("content-length") ? headers["content-length"] : "";
    std::string ct = headers.count("content-type") ? headers["content-type"] : "";
    env_vec.push_back("CONTENT_LENGTH=" + cl);
    env_vec.push_back("CONTENT_TYPE=" + ct);
  }

  char **envp = static_cast<char **>(calloc(env_vec.size() + 1, sizeof(char *)));
  if (!envp) {
    Logger::error(__FILE__, "Allocating memory for envp failed");
    return NULL;
  }

  for (size_t i = 0; i < env_vec.size(); ++i) {
    envp[i] = strdup(env_vec[i].c_str());
    if (!envp[i]) {
      for (size_t j = 0; j < i; ++j)
        free(envp[j]);
      free(envp);
      return NULL;
    }
  }

  return envp;
}

void Cgi::execute(Client *c) {

    const Request *req = c->getRequest();
    Response *resp = c->getResponse();
    if (!req || !resp)  
        return;

  if (pipe(this->pipe_in) < 0) {
    Logger::error(__FILE__, "pipe() failed");
    resp->setStatusCode(500);
    resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
    return;
  }
  if (pipe(this->pipe_out) < 0) {
    Logger::error(__FILE__,  "pipe() failed");
    close(pipe_in[0]);
    close(pipe_in[1]);
    resp->setStatusCode(500);
    resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
    return;
  }
  this->cgi_pid = fork();
  if (this->cgi_pid == 0) {
    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);
    int exit_status = execve(this->args[0], this->args, this->envp);
    exit(exit_status);
  }
  if (this->cgi_pid > 0) {
    close(pipe_in[0]);
    close(pipe_out[1]); 
  } else {
    resp->setStatusCode(500);
    resp->setBody(getErrorPage(resp->getStatus(), c->getServer()));
  }
}


char **Cgi::getArgs() { return this->args; }

void Cgi::setEnv(char **envp) {this->envp = envp;}

char **Cgi::getEnv() const { return this->envp; }

void Cgi::setCGIState(int state) { this->cgi_state = state; }

int Cgi::getCGIState() { return this->cgi_state; }

int *Cgi::getPipeIn() { return this->pipe_in; }

int *Cgi::getPipeOut() { return this->pipe_out; }

int Cgi::getCgiPid() { return this->cgi_pid; }

size_t Cgi::getBytesSended(){ return this->bytes_sended; }

void Cgi::incrementBytesSended(size_t bytes){ this->bytes_sended +=  bytes; }

void Cgi::reset() {
    if (args) {
        for (int i = 0; args[i] != NULL; ++i) {
            free(args[i]);
        }
        free(args);
        args = NULL;
    }

    if (envp && *envp) {
        for (int i = 0; envp[i] != NULL; ++i) {
            free(envp[i]);
        }
        free(envp);
        envp = NULL;
    }

    cgi_state = 0;
    cgi_pid = -1;
    bytes_sended = 0;
    pid = -1;
}
Cgi::Cgi(){
  this->envp = NULL;
  this->args = NULL;
  this->cgi_state = 0;
  this->bytes_sended = 0;
  cgi_pid = -1;
  pid = -1;
};
Cgi::~Cgi() {reset();}

