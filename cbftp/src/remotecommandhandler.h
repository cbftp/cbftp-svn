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
  int sockid;
  bool retrying;
  bool connected;
  void connect();
  void disconnect();
  void handleMessage(std::string);
  void stopRetry();
  void commandRace(const std::string &);
  void commandPrepare(const std::string &);
  void commandRaw(const std::string &);
  void commandFXP(const std::string &);
  void commandDownload(const std::string &);
  void commandUpload(const std::string &);
  void parseRace(const std::string &, bool);
public:
  RemoteCommandHandler();
  bool isEnabled() const;
  int getUDPPort() const;
  std::string getPassword() const;
  void setPassword(std::string);
  void setPort(int);
  void setEnabled(bool);
  void FDData(int, char *, unsigned int);
  void FDFail(int, std::string);
  void tick(int);
};
