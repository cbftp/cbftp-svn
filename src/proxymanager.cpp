#include "proxymanager.h"

#include <algorithm>

#include "proxy.h"
#include "eventlog.h"
#include "util.h"

ProxyManager::ProxyManager() {
  defaultproxy = NULL;
}

void ProxyManager::addProxy(Proxy * proxy) {
  proxies.push_back(proxy);
  sortProxys();
}

Proxy * ProxyManager::getProxy(std::string name) const {
  for (std::vector<Proxy *>::const_iterator it = proxies.begin(); it != proxies.end(); it++) {
    if ((*it)->getName() == name) {
      return *it;
    }
  }
  return NULL;
}

void ProxyManager::removeProxy(std::string name) {
  for (std::vector<Proxy *>::iterator it = proxies.begin(); it != proxies.end(); it++) {
    if ((*it)->getName() == name) {
      delete *it;
      proxies.erase(it);
      return;
    }
  }
}

std::vector<Proxy *>::const_iterator ProxyManager::begin() const {
  return proxies.begin();
}

std::vector<Proxy *>::const_iterator ProxyManager::end() const {
  return proxies.end();
}

bool proxyNameComparator(Proxy * a, Proxy * b) {
  return a->getName().compare(b->getName()) < 0;
}

bool ProxyManager::hasDefaultProxy() const {
  return defaultproxy != NULL;
}

Proxy * ProxyManager::getDefaultProxy() const {
  return defaultproxy;
}

void ProxyManager::setDefaultProxy(std::string proxy) {
  defaultproxy = getProxy(proxy);
}

void ProxyManager::sortProxys() {
  std::sort(proxies.begin(), proxies.end(), proxyNameComparator);
}

unsigned int ProxyManager::size() const {
  return proxies.size();
}
