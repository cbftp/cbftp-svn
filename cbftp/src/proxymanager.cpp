#include "proxymanager.h"

#include <algorithm>

#include "proxy.h"
#include "globalcontext.h"
#include "datafilehandler.h"
#include "eventlog.h"
#include "util.h"

extern GlobalContext * global;

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

void ProxyManager::readConfiguration() {
  std::vector<std::string> lines;
  global->getDataFileHandler()->getDataFor("ProxyManager", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok1 = line.find('$');
    size_t tok2 = line.find('=', tok1);
    std::string name = line.substr(0, tok1);
    std::string setting = line.substr(tok1 + 1, (tok2 - tok1 - 1));
    std::string value = line.substr(tok2 + 1);
    Proxy * proxy = getProxy(name);
    if (proxy == NULL) {
      proxy = new Proxy(name);
      proxies.push_back(proxy);
    }
    if (!setting.compare("addr")) {
      proxy->setAddr(value);
    }
    else if (!setting.compare("port")) {
      proxy->setPort(value);
    }
    else if (!setting.compare("authmethod")) {
      proxy->setAuthMethod(util::str2Int(value));
    }
    else if (!setting.compare("user")) {
      proxy->setUser(value);
    }
    else if (!setting.compare("pass")) {
      proxy->setPass(value);
    }
  }
  lines.clear();
  global->getDataFileHandler()->getDataFor("ProxyManagerDefaults", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("useproxy")) {
      setDefaultProxy(value);
    }
  }
  std::sort(proxies.begin(), proxies.end(), proxyNameComparator);
  global->getEventLog()->log("ProxyManager", "Loaded " + util::int2Str((int)proxies.size()) + " proxies.");
}

void ProxyManager::writeState() {
  global->getEventLog()->log("ProxyManager", "Writing state...");
  std::vector<Proxy *>::iterator it;
  std::string filetag = "ProxyManager";
  std::string defaultstag = "ProxyManagerDefaults";
  DataFileHandler * filehandler = global->getDataFileHandler();
  for (it = proxies.begin(); it != proxies.end(); it++) {
    Proxy * proxy = *it;
    std::string name = proxy->getName();
    filehandler->addOutputLine(filetag, name + "$addr=" + proxy->getAddr());
    filehandler->addOutputLine(filetag, name + "$port=" + proxy->getPort());
    filehandler->addOutputLine(filetag, name + "$user=" + proxy->getUser());
    filehandler->addOutputLine(filetag, name + "$pass=" + proxy->getPass());
    filehandler->addOutputLine(filetag, name + "$authmethod=" + util::int2Str(proxy->getAuthMethod()));
  }
  if (defaultproxy != NULL) {
    filehandler->addOutputLine(defaultstag, "useproxy=" + defaultproxy->getName());
  }
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
