#pragma once

#include <string>

#define PROXY_AUTH_NONE 232
#define PROXY_AUTH_USERPASS 233

class Proxy {
public:
  Proxy();
  Proxy(std::string);
  std::string getName();
  std::string getAddr();
  std::string getPort();
  int getAuthMethod();
  std::string getAuthMethodText();
  std::string getUser();
  std::string getPass();
  void setName(std::string);
  void setAddr(std::string);
  void setPort(std::string);
  void setAuthMethod(int);
  void setUser(std::string);
  void setPass(std::string);
private:
  std::string name;
  std::string addr;
  std::string port;
  int authmethod;
  std::string user;
  std::string pass;
};
