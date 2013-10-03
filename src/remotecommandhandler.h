#pragma once

#include <string>

#include "eventreceiver.h"

#define DEFAULTPORT 55477
#define DEFAULTPASS "DEFAULT"

class GlobalContext;

extern GlobalContext * global;

class RemoteCommandHandler : private EventReceiver {
private:
  bool enabled;
  std::string password;
  int port;
  int sockfd;
  void connect();
  void disconnect();
  void handleMessage(std::string);
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
  void FDData(char *, unsigned int);
};
