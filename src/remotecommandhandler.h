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
  bool isEnabled() const;
  int getUDPPort() const;
  std::string getPassword() const;
  void setPassword(std::string);
  void setPort(int);
  void setEnabled(bool);
  void readConfiguration();
  void writeState();
  void FDData(char *, unsigned int);
};
