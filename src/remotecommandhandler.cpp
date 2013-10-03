#include "remotecommandhandler.h"

#include <vector>
#include <list>

#include "globalcontext.h"
#include "datafilehandler.h"
#include "engine.h"
#include "iomanager.h"

RemoteCommandHandler::RemoteCommandHandler() {
  enabled = false;
  password = DEFAULTPASS;
  port = DEFAULTPORT;
}

bool RemoteCommandHandler::isEnabled() {
  return enabled;
}

int RemoteCommandHandler::getUDPPort() {
  return port;
}

std::string RemoteCommandHandler::getPassword() {
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
  sockfd = global->getIOManager()->registerUDPServerSocket(this, getUDPPort());
}

void RemoteCommandHandler::FDData(char * data, unsigned int datalen) {
  handleMessage(std::string(data, datalen));
}

void RemoteCommandHandler::handleMessage(std::string message) {
  size_t one = message.find(" ");
  size_t two = message.find(" ", one + 1);
  size_t three = message.find(" ", two + 1);
  if (one == std::string::npos || two == std::string::npos || three == std::string::npos) {
    return;
  }
  std::string pass = message.substr(0, one);
  if (pass != password) {
    return;
  }
  std::string section = message.substr(one + 1, two - (one + 1));
  std::string release = message.substr(two + 1, three - (two + 1));
  std::string sitestring = message.substr(three + 1);
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
  global->getEngine()->newRace(release, section, sites);
}

void RemoteCommandHandler::disconnect() {
  global->getIOManager()->closeSocket(sockfd);
}

void RemoteCommandHandler::setEnabled(bool enabled) {
  if ((isEnabled() && enabled) || (!isEnabled() && !enabled)) {
    return;
  }
  if (enabled) {
    connect();
  }
  else {
    disconnect();
  }
  this->enabled = enabled;
}

void RemoteCommandHandler::readConfiguration() {
  std::vector<std::string> lines;
  global->getDataFileHandler()->getDataFor("RemoteCommandHandler", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
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
      setPort(global->str2Int(value));
    }
    else if (!setting.compare("password")) {
      setPassword(value);
    }
  }
  if (enable) {
    setEnabled(true);
  }
}

void RemoteCommandHandler::writeState() {
  DataFileHandler * filehandler = global->getDataFileHandler();
  if (enabled) filehandler->addOutputLine("RemoteCommandHandler", "enabled=true");
  filehandler->addOutputLine("RemoteCommandHandler", "port=" + global->int2Str(port));
  filehandler->addOutputLine("RemoteCommandHandler", "password=" + password);
}
