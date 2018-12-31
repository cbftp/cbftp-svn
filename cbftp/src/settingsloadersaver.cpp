#include "settingsloadersaver.h"

#include <vector>
#include <set>

#include "core/tickpoke.h"
#include "core/iomanager.h"
#include "core/sslmanager.h"
#include "crypto.h"
#include "globalcontext.h"
#include "datafilehandler.h"
#include "eventlog.h"
#include "remotecommandhandler.h"
#include "externalfileviewing.h"
#include "skiplist.h"
#include "skiplistitem.h"
#include "proxymanager.h"
#include "proxy.h"
#include "localstorage.h"
#include "uibase.h"
#include "sitemanager.h"
#include "site.h"
#include "path.h"
#include "engine.h"
#include "hourlyalltracking.h"
#include "sectionmanager.h"

#define AUTO_SAVE_INTERVAL 600000 // 10 minutes

SettingsLoaderSaver::SettingsLoaderSaver() :
  dfh(std::make_shared<DataFileHandler>()){

}

bool SettingsLoaderSaver::enterKey(const std::string & key) {
  if (dfh->isInitialized()) {
    return false;
  }
  if (dataExists()) {
    bool result = dfh->readEncrypted(key);
    if (result) {
      global->getEventLog()->log("DataLoaderSaver", "Data decryption successful.");
      loadSettings();
      startAutoSaver();
      return true;
    }
    global->getEventLog()->log("DataLoaderSaver", "Data decryption failed.");
    return false;
  }
  else {
    dfh->newDataFile(key);
    startAutoSaver();
    return true;
  }
}

bool SettingsLoaderSaver::dataExists() const {
  return dfh->fileExists();
}

bool SettingsLoaderSaver::changeKey(const std::string & key, const std::string & newkey) {
  return dfh->changeKey(key, newkey);
}

void SettingsLoaderSaver::loadSettings() {
  std::vector<std::string> lines;
  dfh->getDataFor("IOManager", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("defaultinterface")) {
      global->getIOManager()->setDefaultInterface(value);
    }
  }

  dfh->getDataFor("SSLManager", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("certificate") && value.size()) {
      BinaryData data;
      Crypto::base64Decode(BinaryData(value.begin(), value.end()), data);
      SSLManager::setCertificate(data);
    }
    if (!setting.compare("privatekey") && value.size()) {
      BinaryData data;
      Crypto::base64Decode(BinaryData(value.begin(), value.end()), data);
      SSLManager::setPrivateKey(data);
    }
  }

  dfh->getDataFor("RemoteCommandHandler", &lines);
  bool enable = false;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("enabled")) {
      if (!value.compare("true")) {
        enable = true;
      }
    }
    else if (!setting.compare("port")) {
      global->getRemoteCommandHandler()->setPort(std::stoi(value));
    }
    else if (!setting.compare("password")) {
      global->getRemoteCommandHandler()->setPassword(value);
    }
    else if (!setting.compare("notify")) {
      // begin backward compatibility r977
      if (!value.compare("true")) {
        global->getRemoteCommandHandler()->setNotify(RemoteCommandNotify::ALL_COMMANDS);
        continue;
      }
      // end backward compatibility r977
      global->getRemoteCommandHandler()->setNotify(static_cast<RemoteCommandNotify>(std::stoi(value)));
    }
  }
  if (enable) {
    global->getRemoteCommandHandler()->setEnabled(true);
  }

  dfh->getDataFor("ExternalFileViewing", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("video")) {
      global->getExternalFileViewing()->setVideoViewer(value);
    }
    else if (!setting.compare("audio")) {
      global->getExternalFileViewing()->setAudioViewer(value);
    }
    else if (!setting.compare("image")) {
      global->getExternalFileViewing()->setImageViewer(value);
    }
    else if (!setting.compare("pdf")) {
      global->getExternalFileViewing()->setPDFViewer(value);
    }
  }

  global->getSkipList()->clearEntries();
  dfh->getDataFor("SkipList", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("entry")) {
      loadSkipListEntry(global->getSkipList(), value);
    }
    if (!setting.compare("defaultallow")) {
      global->getSkipList()->setDefaultAllow(value.compare("true") == 0 ? true : false);
    }
  }


  dfh->getDataFor("ProxyManager", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok1 = line.find('$');
    size_t tok2 = line.find('=', tok1);
    std::string name = line.substr(0, tok1);
    std::string setting = line.substr(tok1 + 1, (tok2 - tok1 - 1));
    std::string value = line.substr(tok2 + 1);
    Proxy * proxy = global->getProxyManager()->getProxy(name);
    if (proxy == NULL) {
      proxy = new Proxy(name);
      global->getProxyManager()->addProxy(proxy);
    }
    if (!setting.compare("addr")) {
      proxy->setAddr(value);
    }
    else if (!setting.compare("port")) {
      proxy->setPort(value);
    }
    else if (!setting.compare("authmethod")) {
      proxy->setAuthMethod(std::stoi(value));
    }
    else if (!setting.compare("user")) {
      proxy->setUser(value);
    }
    else if (!setting.compare("pass")) {
      proxy->setPass(value);
    }
  }
  lines.clear();
  dfh->getDataFor("ProxyManagerDefaults", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("useproxy")) {
      global->getProxyManager()->setDefaultProxy(value);
    }
  }
  global->getProxyManager()->sortProxys();

  dfh->getDataFor("LocalStorage", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("temppath")) {
      global->getLocalStorage()->setTempPath(value);
    }
    else if (!setting.compare("downloadpath")) {
      global->getLocalStorage()->setDownloadPath(value);
    }
    else if (!setting.compare("useactivemodeaddr")) {
      if (!value.compare("true")) global->getLocalStorage()->setUseActiveModeAddress(true);
    }
    else if (!setting.compare("activemodeaddr")) {
      global->getLocalStorage()->setActiveModeAddress(value);
    }
    else if (!setting.compare("activeportfirst")) {
      global->getLocalStorage()->setActivePortFirst(std::stoi(value));
    }
    else if (!setting.compare("activeportlast")) {
      global->getLocalStorage()->setActivePortLast(std::stoi(value));
    }
  }

  std::list<std::pair<std::string, std::string> > exceptsources;
  std::list<std::pair<std::string, std::string> > excepttargets;
  dfh->getDataFor("SiteManager", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok1 = line.find('$');
    size_t tok2 = line.find('=', tok1);
    std::string name = line.substr(0, tok1);
    std::string setting = line.substr(tok1 + 1, (tok2 - tok1 - 1));
    std::string value = line.substr(tok2 + 1);
    std::shared_ptr<Site> site = global->getSiteManager()->getSite(name);
    if (!site) {
      site = std::make_shared<Site>(name);
      global->getSiteManager()->addSiteLoad(site);
    }
    if (!setting.compare("addr")) {
      site->setAddresses(value);
    }
    else if (!setting.compare("port")) { // legacy
      site->setAddresses(site->getAddress() + ":" + value);
    }
    else if (!setting.compare("user")) {
      site->setUser(value);
    }
    else if (!setting.compare("pass")) {
      site->setPass(value);
    }
    else if (!setting.compare("basepath")) {
      site->setBasePath(value);
    }
    else if (!setting.compare("idletime")) {
      site->setMaxIdleTime(std::stoi(value));
    }
    else if (!setting.compare("pret")) {
      if (!value.compare("true")) site->setPRET(true);
    }
    else if (!setting.compare("binary")) {
      if (!value.compare("true")) site->setForceBinaryMode(true);
    }
    else if (!setting.compare("xdupe")) {
      if (!value.compare("false")) site->setUseXDUPE(false);
    }
    // begin backward compatibility r938
    else if (!setting.compare("sslconn")) {
      if (!value.compare("false")) site->setTLSMode(TLSMode::NONE);
    }
    // end backward compatibility r938
    else if (!setting.compare("tlsmode")) {
      site->setTLSMode(static_cast<TLSMode>(std::stoi(value)));
    }
    else if (!setting.compare("sslfxpforced")) {
      if (!value.compare("true")) site->setSSLTransferPolicy(SITE_SSL_ALWAYS_ON);
    }
    else if (!setting.compare("ssltransfer")) {
      site->setSSLTransferPolicy(std::stoi(value));
    }
    else if (!setting.compare("sscn")) {
      if (!value.compare("false")) site->setSupportsSSCN(false);
    }
    else if (!setting.compare("cpsv")) {
      if (!value.compare("false")) site->setSupportsCPSV(false);
    }
    else if (!setting.compare("listcommand")) {
      site->setListCommand(std::stoi(value));
    }
    else if (!setting.compare("disabled")) {
      if (!value.compare("true")) site->setDisabled(true);
    }
    else if (!setting.compare("allowupload")) {
      // compatibility: 889 and below
      if (!value.compare("false")) {
        site->setAllowUpload(SITE_ALLOW_TRANSFER_NO);
      }
      // compatibility end
      else {
        site->setAllowUpload(static_cast<SiteAllowTransfer>(std::stoi(value)));
      }
    }
    else if (!setting.compare("allowdownload")) {
      // compatibility: 889 and below
      if (!value.compare("false")) {
        site->setAllowDownload(SITE_ALLOW_TRANSFER_NO);
      }
      // compatibility end
      else {
        site->setAllowDownload(static_cast<SiteAllowTransfer>(std::stoi(value)));
      }
    }
    else if (!setting.compare("priority")) {
      site->setPriority(static_cast<SitePriority>(std::stoi(value)));
    }
    else if (!setting.compare("brokenpasv")) {
      if (!value.compare("true")) site->setBrokenPASV(true);
    }
    else if (!setting.compare("logins")) {
      site->setMaxLogins(std::stoi(value));
    }
    else if (!setting.compare("maxdn")) {
      site->setMaxDn(std::stoi(value));
    }
    else if (!setting.compare("maxup")) {
      site->setMaxUp(std::stoi(value));
    }
    else if (!setting.compare("section")) {
      size_t split = value.find('$');
      std::string sectionname = value.substr(0, split);
      std::string sectionpath = value.substr(split + 1);
      site->addSection(sectionname, sectionpath);
    }
    else if (!setting.compare("avgspeed")) {
      size_t split = value.find('$');
      std::string sitename = value.substr(0, split);
      int avgspeed = std::stoi(value.substr(split + 1));
      site->setAverageSpeed(sitename, avgspeed);
    }
    else if (!setting.compare("affil")) {
      site->addAffil(value);
    }
    else if (!setting.compare("proxytype")) {
      site->setProxyType(std::stoi(value));
    }
    else if (!setting.compare("proxyname")) {
      site->setProxy(value);
    }
    else if (!setting.compare("transfersourcepolicy")) {
      site->setTransferSourcePolicy(std::stoi(value));
    }
    else if (!setting.compare("transfertargetpolicy")) {
      site->setTransferTargetPolicy(std::stoi(value));
    }
    else if (!setting.compare("exceptsourcesite")) {
      exceptsources.push_back(std::pair<std::string, std::string>(name, value));
    }
    else if (!setting.compare("excepttargetsite")) {
      excepttargets.push_back(std::pair<std::string, std::string>(name, value));
    }
    else if (!setting.compare("skiplistentry")) {
      loadSkipListEntry((SkipList *)&site->getSkipList(), value);
    }
    else if (!setting.compare("sizeup")) {
      site->setSizeUp(std::stoll(value));
    }
    else if (!setting.compare("filesup")) {
      site->setFilesUp(std::stoi(value));
    }
    else if (!setting.compare("sizedown")) {
      site->setSizeDown(std::stoll(value));
    }
    else if (!setting.compare("filesdown")) {
      site->setFilesDown(std::stoi(value));
    }
    else if (!setting.compare("sitessizeup")) {
      size_t split = value.find('$');
      std::string sitename = value.substr(0, split);
      unsigned long long int size = std::stoll(value.substr(split + 1));
      site->setSizeUp(sitename, size);
    }
    else if (!setting.compare("sitesfilesup")) {
      size_t split = value.find('$');
      std::string sitename = value.substr(0, split);
      unsigned int files = std::stoi(value.substr(split + 1));
      site->setFilesUp(sitename, files);
    }
    else if (!setting.compare("sitessizedown")) {
      size_t split = value.find('$');
      std::string sitename = value.substr(0, split);
      unsigned long long int size = std::stoll(value.substr(split + 1));
      site->setSizeDown(sitename, size);
    }
    else if (!setting.compare("sitesfilesdown")) {
      size_t split = value.find('$');
      std::string sitename = value.substr(0, split);
      unsigned int files = std::stoi(value.substr(split + 1));
      site->setFilesDown(sitename, files);
    }
  }
  for (std::list<std::pair<std::string, std::string> >::const_iterator it2 = exceptsources.begin(); it2 != exceptsources.end(); it2++) {
    std::shared_ptr<Site> site = global->getSiteManager()->getSite(it2->first);
    std::shared_ptr<Site> except = global->getSiteManager()->getSite(it2->second);
    if (!!site && !!except) {
      site->addExceptSourceSite(except);
    }
  }
  for (std::list<std::pair<std::string, std::string> >::const_iterator it2 = excepttargets.begin(); it2 != excepttargets.end(); it2++) {
    std::shared_ptr<Site> site = global->getSiteManager()->getSite(it2->first);
    std::shared_ptr<Site> except = global->getSiteManager()->getSite(it2->second);
    if (!!site && !!except) {
      site->addExceptTargetSite(except);
    }
  }

  dfh->getDataFor("SiteManagerDefaults", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("username")) {
      global->getSiteManager()->setDefaultUserName(value);
    }
    else if (!setting.compare("password")) {
      global->getSiteManager()->setDefaultPassword(value);
    }
    else if (!setting.compare("maxlogins")) {
      global->getSiteManager()->setDefaultMaxLogins(std::stoi(value));
    }
    else if (!setting.compare("maxup")) {
      global->getSiteManager()->setDefaultMaxUp(std::stoi(value));
    }
    else if (!setting.compare("maxdown")) {
      global->getSiteManager()->setDefaultMaxDown(std::stoi(value));
    }
    // begin backward compatibility r938
    else if (!setting.compare("sslconn")) {
      if (!value.compare("false")) {
        global->getSiteManager()->setDefaultTLSMode(TLSMode::NONE);
      }
    }
    // end backward compatibility r938
    else if (!setting.compare("tlsmode")) {
      global->getSiteManager()->setDefaultTLSMode(static_cast<TLSMode>(std::stoi(value)));
    }
    else if (!setting.compare("sslfxpforced")) {
      if (!value.compare("true")) {
        global->getSiteManager()->setDefaultSSLTransferPolicy(SITE_SSL_ALWAYS_ON);
      }
    }
    else if (!setting.compare("ssltransfer")) {
      global->getSiteManager()->setDefaultSSLTransferPolicy(std::stoi(value));
    }
    else if (!setting.compare("maxidletime")) {
      global->getSiteManager()->setDefaultMaxIdleTime(std::stoi(value));
    }
  }

  dfh->getDataFor("Engine", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("preparedraceexpirytime")) {
      global->getEngine()->setPreparedRaceExpiryTime(std::stoi(value));
    }
  }

  dfh->getDataFor("SectionManager", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok1 = line.find('$');
    size_t tok2 = line.find('=', tok1);
    std::string name = line.substr(0, tok1);
    std::string setting = line.substr(tok1 + 1, (tok2 - tok1 - 1));
    std::string value = line.substr(tok2 + 1);
    Section * section = global->getSectionManager()->getSection(name);
    if (!section) {
      global->getSectionManager()->addSection(name);
      section = global->getSectionManager()->getSection(name);
    }
    if (!setting.compare("jobs")) {
      section->setNumJobs(std::stoi(value));
    }
    if (!setting.compare("skiplistentry")) {
      loadSkipListEntry(&section->getSkipList(), value);
    }
  }

  dfh->getDataFor("Statistics", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("sizeup")) {
      global->getStatistics()->setSizeUp(std::stoll(value));
    }
    else if (!setting.compare("filesup")) {
      global->getStatistics()->setFilesUp(std::stoi(value));
    }
    else if (!setting.compare("sizedown")) {
      global->getStatistics()->setSizeDown(std::stoll(value));
    }
    else if (!setting.compare("filesdown")) {
      global->getStatistics()->setFilesDown(std::stoi(value));
    }
    else if (!setting.compare("sizefxp")) {
      global->getStatistics()->setSizeFXP(std::stoll(value));
    }
    else if (!setting.compare("filesfxp")) {
      global->getStatistics()->setFilesFXP(std::stoi(value));
    }
    else if (!setting.compare("spreadjobs")) {
      global->getStatistics()->setSpreadJobs(std::stoi(value));
    }
    else if (!setting.compare("transferjobs")) {
      global->getStatistics()->setTransferJobs(std::stoi(value));
    }
  }

  global->getSiteManager()->sortSites();

  for (std::list<SettingsAdder *>::iterator it = settingsadders.begin(); it != settingsadders.end(); it++) {
    (*it)->loadSettings(dfh);
  }
}

void SettingsLoaderSaver::saveSettings() {
  if (!dfh->isInitialized()) {
    return;
  }

  dfh->clearOutputData();

  if (global->getIOManager()->hasDefaultInterface()) {
    dfh->addOutputLine("IOManager", "defaultinterface=" + global->getIOManager()->getDefaultInterface());
  }

  if (SSLManager::hasPrivateKey()) {
    BinaryData data;
    Crypto::base64Encode(SSLManager::privateKey(), data);
    dfh->addOutputLine("SSLManager", "privatekey=" + std::string(data.begin(), data.end()));
  }
  if (SSLManager::hasCertificate()) {
    BinaryData data;
    Crypto::base64Encode(SSLManager::certificate(), data);
    dfh->addOutputLine("SSLManager", "certificate=" + std::string(data.begin(), data.end()));
  }

  if (global->getRemoteCommandHandler()->isEnabled()) dfh->addOutputLine("RemoteCommandHandler", "enabled=true");
  dfh->addOutputLine("RemoteCommandHandler", "port=" + std::to_string(global->getRemoteCommandHandler()->getUDPPort()));
  dfh->addOutputLine("RemoteCommandHandler", "password=" + global->getRemoteCommandHandler()->getPassword());
  dfh->addOutputLine("RemoteCommandHandler", "notify=" + std::to_string(static_cast<int>(global->getRemoteCommandHandler()->getNotify())));

  {
    std::string filetag = "ExternalFileViewing";
    dfh->addOutputLine(filetag, "video=" + global->getExternalFileViewing()->getVideoViewer());
    dfh->addOutputLine(filetag, "audio=" + global->getExternalFileViewing()->getAudioViewer());
    dfh->addOutputLine(filetag, "image=" + global->getExternalFileViewing()->getImageViewer());
    dfh->addOutputLine(filetag, "pdf=" + global->getExternalFileViewing()->getPDFViewer());
  }

  {
    addSkipList(global->getSkipList(), "SkipList", "entry=");
    std::string defaultallowstr = global->getSkipList()->defaultAllow() ? "true" : "false";
    dfh->addOutputLine("SkipList", "defaultallow=" + defaultallowstr);
  }

  {
    std::vector<Proxy *>::const_iterator it;
    std::string filetag = "ProxyManager";
    std::string defaultstag = "ProxyManagerDefaults";
    for (it = global->getProxyManager()->begin(); it != global->getProxyManager()->end(); it++) {
      Proxy * proxy = *it;
      std::string name = proxy->getName();
      dfh->addOutputLine(filetag, name + "$addr=" + proxy->getAddr());
      dfh->addOutputLine(filetag, name + "$port=" + proxy->getPort());
      dfh->addOutputLine(filetag, name + "$user=" + proxy->getUser());
      dfh->addOutputLine(filetag, name + "$pass=" + proxy->getPass());
      dfh->addOutputLine(filetag, name + "$authmethod=" + std::to_string(proxy->getAuthMethod()));
    }
    if (global->getProxyManager()->getDefaultProxy() != NULL) {
      dfh->addOutputLine(defaultstag, "useproxy=" + global->getProxyManager()->getDefaultProxy()->getName());
    }
  }

  {
    std::string filetag = "LocalStorage";
    dfh->addOutputLine(filetag, "temppath=" + global->getLocalStorage()->getTempPath().toString());
    dfh->addOutputLine(filetag, "downloadpath=" + global->getLocalStorage()->getDownloadPath().toString());
    if (global->getLocalStorage()->getUseActiveModeAddress()) dfh->addOutputLine(filetag, "useactivemodeaddr=true");
    dfh->addOutputLine(filetag, "activemodeaddr=" + global->getLocalStorage()->getActiveModeAddress());
    dfh->addOutputLine(filetag, "activeportfirst=" + std::to_string(global->getLocalStorage()->getActivePortFirst()));
    dfh->addOutputLine(filetag, "activeportlast=" + std::to_string(global->getLocalStorage()->getActivePortLast()));
  }

  {
    std::vector<std::shared_ptr<Site> >::const_iterator it;
    std::string filetag = "SiteManager";
    std::string defaultstag = "SiteManagerDefaults";
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      const std::shared_ptr<Site> & site = *it;
      std::string name = site->getName();
      dfh->addOutputLine(filetag, name + "$addr=" + site->getAddressesAsString());
      dfh->addOutputLine(filetag, name + "$user=" + site->getUser());
      dfh->addOutputLine(filetag, name + "$pass=" + site->getPass());
      dfh->addOutputLine(filetag, name + "$basepath=" + site->getBasePath().toString());
      dfh->addOutputLine(filetag, name + "$logins=" + std::to_string(site->getInternMaxLogins()));
      dfh->addOutputLine(filetag, name + "$maxup=" + std::to_string(site->getInternMaxUp()));
      dfh->addOutputLine(filetag, name + "$maxdn=" + std::to_string(site->getInternMaxDown()));
      dfh->addOutputLine(filetag, name + "$idletime=" + std::to_string(site->getMaxIdleTime()));
      dfh->addOutputLine(filetag, name + "$tlsmode=" + std::to_string(static_cast<int>(site->getTLSMode())));
      dfh->addOutputLine(filetag, name + "$ssltransfer=" + std::to_string(site->getSSLTransferPolicy()));
      if (!site->supportsSSCN()) dfh->addOutputLine(filetag, name + "$sscn=false");
      if (!site->supportsCPSV()) dfh->addOutputLine(filetag, name + "$cpsv=false");
      dfh->addOutputLine(filetag, name + "$listcommand=" + std::to_string(site->getListCommand()));
      if (site->needsPRET()) dfh->addOutputLine(filetag, name + "$pret=true");
      if (site->forceBinaryMode()) dfh->addOutputLine(filetag, name + "$binary=true");
      if (!site->useXDUPE()) dfh->addOutputLine(filetag, name + "$xdupe=false");
      if (site->getDisabled()) dfh->addOutputLine(filetag, name + "$disabled=true");
      dfh->addOutputLine(filetag, name + "$allowupload=" + std::to_string(site->getAllowUpload()));
      dfh->addOutputLine(filetag, name + "$allowdownload=" + std::to_string(site->getAllowDownload()));
      dfh->addOutputLine(filetag, name + "$priority=" + std::to_string(static_cast<int>(site->getPriority())));
      if (site->hasBrokenPASV()) dfh->addOutputLine(filetag, name + "$brokenpasv=true");
      int proxytype = site->getProxyType();
      dfh->addOutputLine(filetag, name + "$proxytype=" + std::to_string(proxytype));
      if (proxytype == SITE_PROXY_USE) {
        dfh->addOutputLine(filetag, name + "$proxyname=" + site->getProxy());
      }
      std::map<std::string, Path>::const_iterator sit;
      for (sit = site->sectionsBegin(); sit != site->sectionsEnd(); sit++) {
        dfh->addOutputLine(filetag, name + "$section=" + sit->first + "$" + sit->second.toString());
      }
      std::map<std::string, int>::const_iterator sit2;
      for (sit2 = site->avgspeedBegin(); sit2 != site->avgspeedEnd(); sit2++) {
        dfh->addOutputLine(filetag, name + "$avgspeed=" + sit2->first + "$" + std::to_string(sit2->second));
      }
      dfh->addOutputLine(filetag, name + "$sizeup=" + std::to_string(site->getSizeUpAll()));
      dfh->addOutputLine(filetag, name + "$filesup=" + std::to_string(site->getFilesUpAll()));
      dfh->addOutputLine(filetag, name + "$sizedown=" + std::to_string(site->getSizeDownAll()));
      dfh->addOutputLine(filetag, name + "$filesdown=" + std::to_string(site->getFilesDownAll()));
      std::map<std::string, HourlyAllTracking>::const_iterator sit5;
      for (sit5 = site->sizeUpBegin(); sit5 != site->sizeUpEnd(); sit5++) {
        dfh->addOutputLine(filetag, name + "$sitessizeup=" + sit5->first + "$" + std::to_string(sit5->second.getAll()));
      }
      for (sit5 = site->filesUpBegin(); sit5 != site->filesUpEnd(); sit5++) {
        dfh->addOutputLine(filetag, name + "$sitesfilesup=" + sit5->first + "$" + std::to_string(sit5->second.getAll()));
      }
      for (sit5 = site->sizeDownBegin(); sit5 != site->sizeDownEnd(); sit5++) {
        dfh->addOutputLine(filetag, name + "$sitessizedown=" + sit5->first + "$" + std::to_string(sit5->second.getAll()));
      }
      for (sit5 = site->filesDownBegin(); sit5 != site->filesDownEnd(); sit5++) {
        dfh->addOutputLine(filetag, name + "$sitesfilesdown=" + sit5->first + "$" + std::to_string(sit5->second.getAll()));
      }
      std::set<std::string>::const_iterator sit3;
      for (sit3 = site->affilsBegin(); sit3 != site->affilsEnd(); sit3++) {
        dfh->addOutputLine(filetag, name + "$affil=" + *sit3);
      }
      dfh->addOutputLine(filetag, name + "$transfersourcepolicy=" + std::to_string(site->getTransferSourcePolicy()));
      dfh->addOutputLine(filetag, name + "$transfertargetpolicy=" + std::to_string(site->getTransferTargetPolicy()));
      std::set<std::shared_ptr<Site> >::const_iterator sit4;
      for (sit4 = site->exceptSourceSitesBegin(); sit4 != site->exceptSourceSitesEnd(); sit4++) {
        dfh->addOutputLine(filetag, name + "$exceptsourcesite=" + (*sit4)->getName());
      }
      for (sit4 = site->exceptTargetSitesBegin(); sit4 != site->exceptTargetSitesEnd(); sit4++) {
        dfh->addOutputLine(filetag, name + "$excepttargetsite=" + (*sit4)->getName());
      }
      addSkipList((SkipList *)&site->getSkipList(), filetag, name + "$skiplistentry=");
    }
    dfh->addOutputLine(defaultstag, "username=" + global->getSiteManager()->getDefaultUserName());
    dfh->addOutputLine(defaultstag, "password=" + global->getSiteManager()->getDefaultPassword());
    dfh->addOutputLine(defaultstag, "maxlogins=" + std::to_string(global->getSiteManager()->getDefaultMaxLogins()));
    dfh->addOutputLine(defaultstag, "maxup=" + std::to_string(global->getSiteManager()->getDefaultMaxUp()));
    dfh->addOutputLine(defaultstag, "maxdown=" + std::to_string(global->getSiteManager()->getDefaultMaxDown()));
    dfh->addOutputLine(defaultstag, "maxidletime=" + std::to_string(global->getSiteManager()->getDefaultMaxIdleTime()));
    dfh->addOutputLine(defaultstag, "ssltransfer=" + std::to_string(global->getSiteManager()->getDefaultSSLTransferPolicy()));
    dfh->addOutputLine(defaultstag, "tlsmode=" + std::to_string(static_cast<int>(global->getSiteManager()->getDefaultTLSMode())));
  }

  {
    std::string filetag = "SectionManager";
    for (auto it = global->getSectionManager()->begin(); it != global->getSectionManager()->end(); ++it) {
      const Section & section = it->second;
      dfh->addOutputLine(filetag, section.getName() + "$jobs=" + std::to_string(section.getNumJobs()));
      addSkipList(&section.getSkipList(), filetag, section.getName() + "$skiplistentry=");
    }
  }

  {
    dfh->addOutputLine("Engine", "preparedraceexpirytime=" + std::to_string(global->getEngine()->getPreparedRaceExpiryTime()));
  }
  {
    std::string filetag = "Statistics";
    dfh->addOutputLine(filetag, "sizeup=" + std::to_string(global->getStatistics()->getSizeUpAll()));
    dfh->addOutputLine(filetag, "filesup=" + std::to_string(global->getStatistics()->getFilesUpAll()));
    dfh->addOutputLine(filetag, "sizedown=" + std::to_string(global->getStatistics()->getSizeDownAll()));
    dfh->addOutputLine(filetag, "filesdown=" + std::to_string(global->getStatistics()->getFilesDownAll()));
    dfh->addOutputLine(filetag, "sizefxp=" + std::to_string(global->getStatistics()->getSizeFXPAll()));
    dfh->addOutputLine(filetag, "filesfxp=" + std::to_string(global->getStatistics()->getFilesFXPAll()));
    dfh->addOutputLine(filetag, "spreadjobs=" + std::to_string(global->getStatistics()->getSpreadJobs()));
    dfh->addOutputLine(filetag, "transferjobs=" + std::to_string(global->getStatistics()->getTransferJobs()));
  }

  for (std::list<SettingsAdder *>::iterator it = settingsadders.begin(); it != settingsadders.end(); it++) {
    (*it)->saveSettings(dfh);
  }

  dfh->writeFile();
}

void SettingsLoaderSaver::tick(int) {
  saveSettings();
}

void SettingsLoaderSaver::addSettingsAdder(SettingsAdder * sa) {
  bool found = false;
  for (std::list<SettingsAdder *>::iterator it = settingsadders.begin(); it != settingsadders.end(); it++) {
    if (*it == sa) {
      found = true;
      break;
    }
  }
  if (!found) {
    settingsadders.push_back(sa);
    if (dfh->isInitialized()) {
      sa->loadSettings(dfh);
    }
  }
}

void SettingsLoaderSaver::removeSettingsAdder(SettingsAdder * sa) {
  for (std::list<SettingsAdder *>::iterator it = settingsadders.begin(); it != settingsadders.end(); it++) {
    if (*it == sa) {
      settingsadders.erase(it);
      break;
    }
  }
}

void SettingsLoaderSaver::startAutoSaver() {
  global->getTickPoke()->startPoke(this, "SettingsLoaderSaver", AUTO_SAVE_INTERVAL, 0);
}

void SettingsLoaderSaver::addSkipList(const SkipList * skiplist, const std::string & owner, const std::string & entry) {
  std::list<SkiplistItem>::const_iterator it;
  for (it = skiplist->entriesBegin(); it != skiplist->entriesEnd(); it++) {
    std::string entryline = it->matchPattern() + "$" +
        (it->matchFile() ? "true" : "false") + "$" +
        (it->matchDir() ? "true" : "false") + "$" +
        std::to_string(it->matchScope()) + "$" +
        std::to_string(it->getAction());
    dfh->addOutputLine(owner, entry + entryline);
  }
}

void SettingsLoaderSaver::loadSkipListEntry(SkipList * skiplist, std::string value) {
  size_t split = value.find('$');
  if (split != std::string::npos) {
    std::string pattern = value.substr(0, split);
    value = value.substr(split + 1);
    split = value.find('$');
    bool file = value.substr(0, split) == "true" ? true : false;
    value = value.substr(split + 1);
    split = value.find('$');
    bool dir = value.substr(0, split) == "true" ? true : false;
    value = value.substr(split + 1);
    split = value.find('$');
    int scope = std::stoi(value.substr(0, split));
    std::string actionstring = value.substr(split + 1);

    SkipListAction action;
    // begin compatibility r892
    if (actionstring == "true") {
      action = SKIPLIST_ALLOW;
    }
    else if (actionstring == "false") {
      action = SKIPLIST_DENY;
    }
    else
    // end compatibility r892
    action = static_cast<SkipListAction>(std::stoi(actionstring));
    skiplist->addEntry(pattern, file, dir, scope, action);
  }
}
