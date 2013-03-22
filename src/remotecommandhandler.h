#pragma once

#include <string>
#include <vector>

#include "globalcontext.h"
#include "datafilehandler.h"

#define DEFAULTPORT 55477
#define DEFAULTPASS "DEFAULT"

extern GlobalContext * global;

class RemoteCommandHandler {
private:
  bool enabled;
  std::string password;
  int port;
public:
  RemoteCommandHandler();
  bool isEnabled();
  int getUDPPort();
  std::string getPassword();
  void setPassword(std::string);
  void setPort(int);
  void setEnabled(bool);
  void readConfiguration();
  void writeState();
};
