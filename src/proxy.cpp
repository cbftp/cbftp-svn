#include "proxy.h"

Proxy::Proxy() {

}

Proxy::Proxy(std::string name) {
  this->name = name;
  addr = "127.0.0.1";
  port = "8080";
  authmethod = PROXY_AUTH_NONE;
  user = "";
  pass = "";
}

std::string Proxy::getName() const {
  return name;
}

std::string Proxy::getAddr() const {
  return addr;
}

std::string Proxy::getPort() const {
  return port;
}

int Proxy::getAuthMethod() const {
  return authmethod;
}

std::string Proxy::getAuthMethodText() const {
  switch (authmethod) {
    case PROXY_AUTH_NONE:
      return "None";
    case PROXY_AUTH_USERPASS:
      return "User/Pass";
  }
  return "Unknown";
}

std::string Proxy::getUser() const {
  return user;
}

std::string Proxy::getPass() const {
  return pass;
}

void Proxy::setName(std::string name) {
  this->name = name;
}

void Proxy::setAddr(std::string addr) {
  this->addr = addr;
}

void Proxy::setPort(std::string port) {
  this->port = port;
}

void Proxy::setAuthMethod(int authmethod) {
  this->authmethod = authmethod;
}

void Proxy::setUser(std::string user) {
  this->user = user;
}

void Proxy::setPass(std::string pass) {
  this->pass = pass;
}
