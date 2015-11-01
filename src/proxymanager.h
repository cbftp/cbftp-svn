#pragma once

#include <vector>
#include <string>

class Proxy;

class ProxyManager {
public:
  ProxyManager();
  void addProxy(Proxy *);
  Proxy * getProxy(std::string) const;
  void removeProxy(std::string);
  std::vector<Proxy *>::const_iterator begin() const;
  std::vector<Proxy *>::const_iterator end() const;
  bool hasDefaultProxy() const;
  Proxy * getDefaultProxy() const;
  void setDefaultProxy(std::string);
  void sortProxys();
  unsigned int size() const;
private:
  std::vector<Proxy *> proxies;
  Proxy * defaultproxy;
};

bool proxyNameComparator(Proxy *, Proxy *);
