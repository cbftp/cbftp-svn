#include "remotecommandhandler.h"

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

void RemoteCommandHandler::setEnabled(bool enabled) {
  this->enabled = enabled;
  // code to open socket here
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
