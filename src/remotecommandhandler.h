#pragma once

#include <string>

#include "core/eventreceiver.h"

class RemoteCommandHandler : private EventReceiver {
private:
  bool enabled;
  std::string password;
  int port;
  int sockid;
  bool retrying;
  bool connected;
  bool notify;
  void connect();
  void disconnect();
  void handleMessage(std::string);
  void stopRetry();
  void commandRace(const std::string &);
  void commandDistribute(const std::string &);
  void commandPrepare(const std::string &);
  void commandRaw(const std::string &);
  void commandRawWithPath(const std::string &);
  void commandFXP(const std::string &);
  void commandDownload(const std::string &);
  void commandUpload(const std::string &);
  void commandIdle(const std::string &);
  void commandAbort(const std::string &);
  void commandDelete(const std::string &);
  void commandReset(const std::string &);
  void parseRace(const std::string &, int);
public:
  RemoteCommandHandler();
  bool isEnabled() const;
  int getUDPPort() const;
  std::string getPassword() const;
  void setPassword(const std::string &);
  void setPort(int);
  bool getNotify() const;
  void setNotify(bool);
  void setEnabled(bool);
  void FDData(int, char *, unsigned int);
  void FDFail(int, const std::string &);
  void tick(int);
};
