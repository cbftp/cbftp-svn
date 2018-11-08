#include "remotecommandhandler.h"

#include <vector>
#include <list>

#include "core/tickpoke.h"
#include "core/iomanager.h"
#include "globalcontext.h"
#include "engine.h"
#include "eventlog.h"
#include "util.h"
#include "sitelogicmanager.h"
#include "sitelogic.h"
#include "uibase.h"
#include "sitemanager.h"
#include "site.h"
#include "race.h"
#include "localstorage.h"

#define DEFAULTPORT 55477
#define DEFAULTPASS "DEFAULT"
#define RETRYDELAY 30000

enum RaceType {
  RACE,
  DISTRIBUTE,
  PREPARE
};

std::list<std::shared_ptr<SiteLogic> > getSiteLogicList(const std::string & sitestring, bool log = true) {
  std::list<std::shared_ptr<SiteLogic> > sitelogics;
  std::list<std::string> sites;
  if (sitestring == "*") {
    std::vector<std::shared_ptr<Site> >::const_iterator it;
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      if (!(*it)->getDisabled()) {
        sites.push_back((*it)->getName());
      }
    }
  }
  else {
    sites = util::trim(util::split(sitestring, ","));
  }
  std::list<std::string> notfoundsites;
  for (std::list<std::string>::const_iterator it = sites.begin(); it != sites.end(); it++) {
    const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(*it);
    if (!sl) {
      notfoundsites.push_back(*it);
      continue;
    }
    sitelogics.push_back(sl);
  }
  if (!sitelogics.empty() || log) {
    for (std::list<std::string>::const_iterator it = notfoundsites.begin(); it != notfoundsites.end(); ++it) {
      global->getEventLog()->log("RemoteCommandHandler", "Site not found: " + *it);
    }
  }
  return sitelogics;
}

RemoteCommandHandler::RemoteCommandHandler() :
  enabled(false),
  password(DEFAULTPASS),
  port(DEFAULTPORT),
  retrying(false),
  connected(false),
  notify(false) {
}

bool RemoteCommandHandler::isEnabled() const {
  return enabled;
}

int RemoteCommandHandler::getUDPPort() const {
  return port;
}

std::string RemoteCommandHandler::getPassword() const {
  return password;
}

void RemoteCommandHandler::setPassword(const std::string & newpass) {
  password = newpass;
}

void RemoteCommandHandler::setPort(int newport) {
  bool reopen = true;
  if (port == newport || !isEnabled()) {
    reopen = false;
  }
  port = newport;
  if (reopen) {
    setEnabled(false);
    setEnabled(true);
  }
}

bool RemoteCommandHandler::getNotify() const {
  return notify;
}

void RemoteCommandHandler::setNotify(bool notify) {
  this->notify = notify;
}

void RemoteCommandHandler::connect() {
  int udpport = getUDPPort();
  sockid = global->getIOManager()->registerUDPServerSocket(this, udpport);
  if (sockid >= 0) {
    connected = true;
    global->getEventLog()->log("RemoteCommandHandler", "Listening on UDP port " + util::int2Str(udpport));
  }
  else {
    int delay = RETRYDELAY / 1000;
    global->getEventLog()->log("RemoteCommandHandler", "Retrying in " + util::int2Str(delay) + " seconds.");
    retrying = true;
    global->getTickPoke()->startPoke(this, "RemoteCommandHandler", RETRYDELAY, 0);
  }
}

void RemoteCommandHandler::FDData(int sockid, char * data, unsigned int datalen) {
  handleMessage(std::string(data, datalen));
}

void RemoteCommandHandler::handleMessage(const std::string & message) {
  std::string trimmedmessage = util::trim(message);
  std::vector<std::string> tokens = util::splitVec(trimmedmessage);
  if (tokens.size() < 2) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad message format: " + trimmedmessage);
    return;
  }
  std::string & pass = tokens[0];
  bool passok = pass == password;
  if (passok) {
    for (unsigned int i = 0; i < pass.length(); i++) {
      pass[i] = '*';
    }
  }
  global->getEventLog()->log("RemoteCommandHandler", "Received: " + util::join(tokens));
  if (!passok) {
    global->getEventLog()->log("RemoteCommandHandler", "Invalid password.");
    return;
  }
  std::string command = tokens[1];
  std::vector<std::string> remainder(tokens.begin() + 2, tokens.end());
  if (command == "race") {
    commandRace(remainder);
  }
  else if (command == "distribute") {
    commandDistribute(remainder);
  }
  else if (command == "prepare") {
    commandPrepare(remainder);
  }
  else if (command == "raw") {
    commandRaw(remainder);
  }
  else if (command == "rawwithpath") {
    commandRawWithPath(remainder);
  }
  else if (command == "fxp") {
    commandFXP(remainder);
  }
  else if (command == "download") {
    commandDownload(remainder);
  }
  else if (command == "upload") {
    commandUpload(remainder);
  }
  else if (command == "idle") {
    commandIdle(remainder);
  }
  else if (command == "abort") {
    commandAbort(remainder);
  }
  else if (command == "delete") {
    commandDelete(remainder);
  }
  else if(command == "reset") {
    commandReset(remainder, false);
  }
  else if(command == "hardreset") {
      commandReset(remainder, true);
    }
  else {
    global->getEventLog()->log("RemoteCommandHandler", "Invalid remote command: " + util::join(tokens));
    return;
  }
  if (notify) {
    global->getUIBase()->notify();
  }
}

void RemoteCommandHandler::commandRace(const std::vector<std::string> & message) {
  parseRace(message, RACE);
}

void RemoteCommandHandler::commandDistribute(const std::vector<std::string> & message) {
  parseRace(message, DISTRIBUTE);
}

void RemoteCommandHandler::commandPrepare(const std::vector<std::string> & message) {
  parseRace(message, PREPARE);
}

void RemoteCommandHandler::commandRaw(const std::vector<std::string> & message) {
  if (message.size() < 2) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote raw command format: " + util::join(message));
    return;
  }
  std::string sitestring = message[0];
  std::string rawcommand = util::join(std::vector<std::string>(message.begin() + 1, message.end()));

  std::list<std::shared_ptr<SiteLogic> > sites = getSiteLogicList(sitestring);

  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
    (*it)->requestRawCommand(rawcommand);
  }
}

void RemoteCommandHandler::commandRawWithPath(const std::vector<std::string> & message) {
  if (message.size() < 3) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote rawwithpath command format: " + util::join(message));
    return;
  }
  std::string sitestring = message[0];
  std::string pathstr = message[1];
  std::string rawcommand = util::join(std::vector<std::string>(message.begin() + 2, message.end()));

  std::list<std::shared_ptr<SiteLogic> > sites = getSiteLogicList(sitestring);

  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
    Path path(pathstr);
    if (path.isRelative()) {
      if ((*it)->getSite()->hasSection(path.toString())) {
        path = (*it)->getSite()->getSectionPath(path.toString());
      }
      else {
        global->getEventLog()->log("RemoteCommandHandler", "Path must be absolute or a section name on " +
            (*it)->getSite()->getName() + ": " + path.toString());
        return;
      }
    }
    (*it)->requestRawCommand(path, rawcommand, false);
  }
}

void RemoteCommandHandler::commandFXP(const std::vector<std::string> & message) {
  if (message.size() < 5) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote fxp command format: " + util::join(message));
    return;
  }
  std::shared_ptr<SiteLogic> srcsl = global->getSiteLogicManager()->getSiteLogic(message[0]);
  std::shared_ptr<SiteLogic> dstsl = global->getSiteLogicManager()->getSiteLogic(message[3]);
  if (!srcsl) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad site name: " + message[0]);
    return;
  }
  if (!dstsl) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad site name: " + message[3]);
    return;
  }
  std::string dstfile = message.size() > 5 ? message[5] : message[2];
  std::string srcpath = message[1];
  if (Path(srcpath).isRelative()) {
    if (srcsl->getSite()->hasSection(srcpath)) {
      srcpath = srcsl->getSite()->getSectionPath(srcpath).toString();
    }
    else {
      global->getEventLog()->log("RemoteCommandHandler", "Path must be absolute or a section name: " + srcpath);
      return;
    }
  }
  std::string dstpath = message[4];
  if (Path(dstpath).isRelative()) {
    if (dstsl->getSite()->hasSection(dstpath)) {
      dstpath = dstsl->getSite()->getSectionPath(dstpath).toString();
    }
    else {
      global->getEventLog()->log("RemoteCommandHandler", "Path must be absolute or a section name: " + dstpath);
      return;
    }
  }
  global->getEngine()->newTransferJobFXP(message[0], srcpath, message[2], message[3], dstpath, dstfile);
}

void RemoteCommandHandler::commandDownload(const std::vector<std::string> & message) {
  if (message.size() < 2) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad download command format: " + util::join(message));
    return;
  }
  std::shared_ptr<SiteLogic> srcsl = global->getSiteLogicManager()->getSiteLogic(message[0]);
  if (!srcsl) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad site name: " + message[0]);
    return;
  }
  Path srcpath = message[1];
  std::string file = srcpath.baseName();
  if (message.size() == 2) {
    if (srcpath.isRelative()) {
      global->getEventLog()->log("RemoteCommandHandler", "Path must be absolute or a section name followed by file: " + message[1]);
      return;
    }
    srcpath = srcpath.dirName();
  }
  else {
    if (srcpath.isRelative()) {
      if (srcsl->getSite()->hasSection(message[1])) {
        srcpath = srcsl->getSite()->getSectionPath(message[1]);
      }
      else {
        global->getEventLog()->log("RemoteCommandHandler", "Path must be absolute or a section name: " + message[1]);
        return;
      }
    }
    file = message[2];
  }
  global->getEngine()->newTransferJobDownload(message[0], srcpath, file, global->getLocalStorage()->getDownloadPath(), file);
}

void RemoteCommandHandler::commandUpload(const std::vector<std::string> & message) {
  if (message.size() < 3) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad upload command format: " + util::join(message));
    return;
  }
  Path srcpath = message[0];
  std::string file = srcpath.baseName();
  std::string dstsite;
  Path dstpath;
  if (message.size() == 3) {
    srcpath = srcpath.dirName();
    dstsite = message[1];
    dstpath = message[2];
  }
  else {
    file = message[1];
    dstsite = message[2];
    dstpath = message[3];
  }
  std::shared_ptr<SiteLogic> dstsl = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!dstsl) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad site name: " + dstsite);
    return;
  }
  if (dstpath.isRelative()) {
    if (dstsl->getSite()->hasSection(dstpath.toString())) {
      dstpath = dstsl->getSite()->getSectionPath(dstpath.toString());
    }
    else {
      global->getEventLog()->log("RemoteCommandHandler", "Path must be absolute or a section name: " + dstpath.toString());
      return;
    }
  }
  global->getEngine()->newTransferJobUpload(srcpath, file, dstsite, dstpath, file);
}

void RemoteCommandHandler::commandIdle(const std::vector<std::string> & message) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad idle command format: " + util::join(message));
    return;
  }
  int idletime;
  std::string sitestring;
  if (message.size() < 2) {
    sitestring = message[0];
    idletime = 0;
  }
  else {
    sitestring = message[0];
    idletime = util::str2Int(message[1]);
  }

  std::list<std::shared_ptr<SiteLogic> > sites = getSiteLogicList(sitestring, false);

  if (sites.empty()) {
    bool found = false;
    for (std::vector<std::shared_ptr<Site> >::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); ++it) {
      if ((*it)->hasSection(sitestring)) {
        std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic((*it)->getName());
        sl->requestAllIdle((*it)->getSectionPath(sitestring), idletime);
        found = true;
      }
    }
    if (!found) {
      global->getEventLog()->log("RemoteCommandHandler", "Sites or section not found: " + sitestring);
    }
  }
  else {
    for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
      (*it)->requestAllIdle(idletime);
    }
  }
}

void RemoteCommandHandler::commandAbort(const std::vector<std::string> & message) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad abort command format: " + util::join(message));
    return;
  }
  std::shared_ptr<Race> race = global->getEngine()->getRace(message[0]);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + message[0]);
    return;
  }
  global->getEngine()->abortRace(race);
}

void RemoteCommandHandler::commandDelete(const std::vector<std::string> & message) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad delete command format: " + util::join(message));
    return;
  }
  std::string release = message[0];
  std::string sitestring;
  if (message.size() >= 2) {
    sitestring = message[1];
  }
  std::shared_ptr<Race> race = global->getEngine()->getRace(release);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + release);
    return;
  }
  if (!sitestring.length()) {
    global->getEngine()->deleteOnAllSites(race);
    return;
  }
  std::list<std::shared_ptr<SiteLogic> > sitelogics = getSiteLogicList(sitestring);
  std::list<std::shared_ptr<Site> > sites;
  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sitelogics.begin(); it != sitelogics.end(); it++) {
    sites.push_back((*it)->getSite());
  }
  global->getEngine()->deleteOnSites(race, sites);
}

void RemoteCommandHandler::commandReset(const std::vector<std::string> & message, bool hard) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad reset command format: " + util::join(message));
    return;
  }
  std::shared_ptr<Race> race = global->getEngine()->getRace(message[0]);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + message[0]);
    return;
  }
  global->getEngine()->resetRace(race, hard);
}

void RemoteCommandHandler::parseRace(const std::vector<std::string> & message, int type) {
  if (message.size() < 3) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote race command format: " + util::join(message));
    return;
  }
  std::string section = message[0];
  std::string release = message[1];
  std::string sitestring = message[2];
  if (sitestring == "*") {
    if (type == RACE) {
      global->getEngine()->newRace(release, section);
    }
    else if (type == DISTRIBUTE){
      global->getEngine()->newDistribute(release, section);
    }
    else {
      global->getEngine()->prepareRace(release, section);
    }
    return;
  }
  std::list<std::string> sites = util::trim(util::split(sitestring, ","));
  if (type == RACE) {
    global->getEngine()->newRace(release, section, sites);
  }
  else if (type == DISTRIBUTE){
    global->getEngine()->newDistribute(release, section, sites);
  }
  else {
    global->getEngine()->prepareRace(release, section, sites);
  }
}

void RemoteCommandHandler::FDFail(int sockid, const std::string & message) {
  global->getEventLog()->log("RemoteCommandHandler", "UDP binding on port " +
      util::int2Str(getUDPPort()) + " failed: " + message);
}

void RemoteCommandHandler::disconnect() {
  if (connected) {
    global->getIOManager()->closeSocket(sockid);
    global->getEventLog()->log("RemoteCommandHandler", "Closing UDP socket");
    connected = false;
  }
}

void RemoteCommandHandler::setEnabled(bool enabled) {
  if ((isEnabled() && enabled) || (!isEnabled() && !enabled)) {
    return;
  }
  if (retrying) {
    stopRetry();
  }
  if (enabled) {
    connect();
  }
  else {
    disconnect();
  }
  this->enabled = enabled;
}

void RemoteCommandHandler::stopRetry() {
  if (retrying) {
    global->getTickPoke()->stopPoke(this, 0);
    retrying = false;
  }
}

void RemoteCommandHandler::tick(int) {
  stopRetry();
  if (enabled) {
    connect();
  }
}
