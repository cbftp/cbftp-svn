#pragma once

#include <string>

#include "eventreceiver.h"

#define DEFAULTPORT 55477
#define DEFAULTPASS "DEFAULT"
#define RETRYDELAY 30000

class GlobalContext;

extern GlobalContext * global;

class RemoteCommandHandler : private EventReceiver {
private:
  bool enabled;
  std::string password;
  int port;
  int sockfd;
  bool retrying;
  bool connected;
  void connect();
  void disconnect();
  void handleMessage(std::string);
  void stopRetry();
  void commandRace(const std::string &);
  void commandRaw(const std::string &);
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
  void FDFail(std::string);
  void tick(int);
};
