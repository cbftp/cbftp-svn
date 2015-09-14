#pragma once

#include <string>

#define PROXY_AUTH_NONE 232
#define PROXY_AUTH_USERPASS 233

class Proxy {
public:
  Proxy();
  Proxy(std::string);
  std::string getName() const;
  std::string getAddr() const;
  std::string getPort() const;
  int getAuthMethod() const;
  std::string getAuthMethodText() const;
  std::string getUser() const;
  std::string getPass() const;
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
