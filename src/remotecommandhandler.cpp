#include "remotecommandhandler.h"

#include <vector>
#include <list>

#include "globalcontext.h"
#include "engine.h"
#include "iomanager.h"
#include "eventlog.h"
#include "tickpoke.h"
#include "util.h"
#include "sitelogicmanager.h"
#include "sitelogic.h"

RemoteCommandHandler::RemoteCommandHandler() :
  enabled(false),
  password(DEFAULTPASS),
  port(DEFAULTPORT),
  retrying(false),
  connected(false) {
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
  global->getEventLog()->log("RemoteCommandHandler", "Received: " + message);
  size_t passend = message.find(" ");
  if (passend == std::string::npos) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad message format: " + message);
    return;
  }
  std::string pass = message.substr(0, passend);
  if (pass != password) {
    global->getEventLog()->log("RemoteCommandHandler", "Invalid password.");
    return;
  }
  size_t commandend = message.find(" ", passend + 1);
  if (commandend == std::string::npos) {
    commandend = message.length();
  }
  std::string command = message.substr(passend + 1, commandend - (passend + 1));
  std::string remainder = message.substr(commandend + 1);
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
  else {
    global->getEventLog()->log("RemoteCommandHandler", "Invalid remote command: " + message);
    return;
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
  std::list<std::string> sites;
  while (true) {
    size_t commapos = sitestring.find(",");
    if (commapos != std::string::npos) {
      sites.push_back(sitestring.substr(0, commapos));
      sitestring = sitestring.substr(commapos + 1);
    }
    else {
      sites.push_back(sitestring);
      break;
    }
  }
  for (std::list<std::string>::const_iterator it = sites.begin(); it != sites.end(); it++) {
    SiteLogic * sl = global->getSiteLogicManager()->getSiteLogic(*it);
    if (sl == NULL) {
      global->getEventLog()->log("RemoteCommandHandler", "Site not found: " + *it);
      continue;
    }
    sl->requestRawCommand(rawcommand, false);
  }
}

void RemoteCommandHandler::commandFXP(const std::string & message) {

}

void RemoteCommandHandler::commandDownload(const std::string & message) {

}

void RemoteCommandHandler::commandUpload(const std::string & message) {

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
  std::list<std::string> sites;
  while (true) {
    size_t commapos = sitestring.find(",");
    if (commapos != std::string::npos) {
      sites.push_back(sitestring.substr(0, commapos));
      sitestring = sitestring.substr(commapos + 1);
    }
    else {
      sites.push_back(sitestring);
      break;
    }
  }
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
