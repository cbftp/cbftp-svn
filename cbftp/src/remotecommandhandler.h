#pragma once

#include <string>
#include <vector>

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
  void handleMessage(const std::string & message);
  void stopRetry();
  void commandRace(const std::vector<std::string> &);
  void commandDistribute(const std::vector<std::string> &);
  void commandPrepare(const std::vector<std::string> &);
  void commandRaw(const std::vector<std::string> &);
  void commandRawWithPath(const std::vector<std::string> &);
  void commandFXP(const std::vector<std::string> &);
  void commandDownload(const std::vector<std::string> &);
  void commandUpload(const std::vector<std::string> &);
  void commandIdle(const std::vector<std::string> &);
  void commandAbort(const std::vector<std::string> &);
  void commandDelete(const std::vector<std::string> &);
  void commandReset(const std::vector<std::string> &, bool);
  void parseRace(const std::vector<std::string> &, int);
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
