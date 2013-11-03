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

std::string Proxy::getName() {
  return name;
}

std::string Proxy::getAddr() {
  return addr;
}

std::string Proxy::getPort() {
  return port;
}

int Proxy::getAuthMethod() {
  return authmethod;
}

std::string Proxy::getAuthMethodText() {
  switch (authmethod) {
    case PROXY_AUTH_NONE:
      return "None";
    case PROXY_AUTH_USERPASS:
      return "User/Pass";
  }
  return "Unknown";
}

std::string Proxy::getUser() {
  return user;
}

std::string Proxy::getPass() {
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
