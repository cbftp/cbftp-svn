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

#define DEFAULTPORT 55477
#define DEFAULTPASS "DEFAULT"
#define RETRYDELAY 30000

std::list<Pointer<SiteLogic> > getSiteLogicList(const std::string & sitestring) {
  std::list<Pointer<SiteLogic> > sitelogics;
  std::list<std::string> sites;
  if (sitestring == "*") {
    std::vector<Pointer<Site> >::const_iterator it;
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      if (!(*it)->getDisabled()) {
        sites.push_back((*it)->getName());
      }
    }
  }
  else {
    sites = util::split(sitestring, ",");
  }
  for (std::list<std::string>::const_iterator it = sites.begin(); it != sites.end(); it++) {
    const Pointer<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(*it);
    if (!sl) {
      global->getEventLog()->log("RemoteCommandHandler", "Site not found: " + *it);
      continue;
    }
    sitelogics.push_back(sl);
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

void RemoteCommandHandler::setPassword(std::string newpass) {
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

void RemoteCommandHandler::handleMessage(std::string message) {
  message = util::trim(message);
  size_t passend = message.find(" ");
  if (passend == std::string::npos) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad message format: " + message);
    return;
  }
  std::string pass = message.substr(0, passend);
  bool passok = pass == password;
  if (passok) {
    for (unsigned int i = 0; i < passend; i++) {
      message[i] = '*';
    }
  }
  global->getEventLog()->log("RemoteCommandHandler", "Received: " + message);
  if (!passok) {
    global->getEventLog()->log("RemoteCommandHandler", "Invalid password.");
    return;
  }
  size_t commandend = message.find(" ", passend + 1);
  if (commandend == std::string::npos) {
    commandend = message.length();
  }
  std::string command = message.substr(passend + 1, commandend - (passend + 1));
  std::string remainder = message.substr(commandend + (commandend < message.length() ? 1 : 0));
  if (command == "race") {
    commandRace(remainder);
  }
  else if (command == "prepare") {
    commandPrepare(remainder);
  }
  else if (command == "raw") {
    commandRaw(remainder);
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
  else {
    global->getEventLog()->log("RemoteCommandHandler", "Invalid remote command: " + message);
    return;
  }
  if (notify) {
    global->getUIBase()->notify();
  }
}

void RemoteCommandHandler::commandRace(const std::string & message) {
  parseRace(message, true);
}

void RemoteCommandHandler::commandPrepare(const std::string & message) {
  parseRace(message, false);
}

void RemoteCommandHandler::commandRaw(const std::string & message) {
  size_t sitesend = message.find(" ");
  if (sitesend == std::string::npos || sitesend == message.length()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote raw command format: " + message);
    return;
  }
  std::string sitestring = message.substr(0, sitesend);
  std::string rawcommand = message.substr(sitesend + 1);

  std::list<Pointer<SiteLogic> > sites = getSiteLogicList(sitestring);

  for (std::list<Pointer<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
    (*it)->requestRawCommand(rawcommand);
  }
}

void RemoteCommandHandler::commandFXP(const std::string & message) {

}

void RemoteCommandHandler::commandDownload(const std::string & message) {

}

void RemoteCommandHandler::commandUpload(const std::string & message) {

}

void RemoteCommandHandler::commandIdle(const std::string & message) {
  size_t sitesend = message.find(" ");
  int idletime;
  std::string sitestring;
  if (sitesend == std::string::npos || sitesend == message.length()) {
    sitestring = message;
    idletime = 0;
  }
  else {
    idletime = util::str2Int(message.substr(sitesend + 1));
    sitestring = message.substr(0, sitesend);
  }

  std::list<Pointer<SiteLogic> > sites = getSiteLogicList(sitestring);

  for (std::list<Pointer<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
    (*it)->requestAllIdle(idletime);
  }
}

void RemoteCommandHandler::commandAbort(const std::string & message) {
  Pointer<Race> race = global->getEngine()->getRace(message);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + message);
    return;
  }
  global->getEngine()->abortRace(race);
}

void RemoteCommandHandler::commandDelete(const std::string & message) {
  Pointer<Race> race = global->getEngine()->getRace(message);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + message);
    return;
  }
  global->getEngine()->deleteOnAllSites(race);
}

void RemoteCommandHandler::parseRace(const std::string & message, bool autostart) {
  size_t sectionend = message.find(" ");
  size_t releaseend = message.find(" ", sectionend + 1);
  if (sectionend == std::string::npos || releaseend == std::string::npos || releaseend == message.length()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote race command format: " + message);
    return;
  }
  std::string section = message.substr(0, sectionend);
  std::string release = message.substr(sectionend + 1, releaseend - (sectionend + 1));
  std::string sitestring = message.substr(releaseend + 1);
  if (sitestring == "*") {
    if (autostart) {
      global->getEngine()->newRace(release, section);
    }
    else {
      global->getEngine()->prepareRace(release, section);
    }
    return;
  }
  std::list<std::string> sites = util::split(sitestring, ",");
  if (autostart) {
    global->getEngine()->newRace(release, section, sites);
  }
  else {
    global->getEngine()->prepareRace(release, section, sites);
  }
}

void RemoteCommandHandler::FDFail(int sockid, std::string message) {
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
