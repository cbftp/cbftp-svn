#pragma once

#include <string>

#define PROXY_AUTH_NONE 232
#define PROXY_AUTH_USERPASS 233

class Proxy {
public:
  Proxy();
  Proxy(const std::string& name);
  std::string getName() const;
  std::string getAddr() const;
  std::string getPort() const;
  int getAuthMethod() const;
  std::string getAuthMethodText() const;
  std::string getUser() const;
  std::string getPass() const;
  bool getResolveHosts() const;
  void setName(const std::string& name);
  void setAddr(const std::string& addr);
  void setPort(const std::string& port);
  void setAuthMethod(int authmethod);
  void setUser(const std::string& user);
  void setPass(const std::string& pass);
  void setResolveHosts(bool resolvehosts);
private:
  std::string name;
  std::string addr;
  std::string port;
  int authmethod;
  std::string user;
  std::string pass;
  bool resolvehosts;
};
