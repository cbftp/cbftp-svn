#pragma once

#include <vector>
#include <string>

class Proxy;

class ProxyManager {
public:
  ProxyManager();
  void addProxy(Proxy *);
  Proxy * getProxy(std::string);
  void removeProxy(std::string);
  std::vector<Proxy *>::iterator begin();
  std::vector<Proxy *>::iterator end();
  void readConfiguration();
  void writeState();
  bool hasDefaultProxy();
  Proxy * getDefaultProxy();
  void setDefaultProxy(std::string);
  void sortProxys();
  unsigned int size();
private:
  std::vector<Proxy *> proxies;
  Proxy * defaultproxy;
};

bool proxyNameComparator(Proxy *, Proxy *);
