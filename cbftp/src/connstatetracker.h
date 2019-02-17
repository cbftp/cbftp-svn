#pragma once

#include <memory>
#include <string>
#include <list>

#include "delayedcommand.h"

#define CST_DOWNLOAD 981
#define CST_UPLOAD 982
#define CST_LIST 983

class CommandOwner;
class SiteRace;
class FileList;
class TransferMonitor;
class DelayedCommand;
class RecursiveCommandLogic;
class SiteLogicRequest;

class ConnStateTracker {
private:
  unsigned long long int time;
  int idletime;
  int lastcheckedcount;
  std::shared_ptr<SiteRace> lastchecked;
  DelayedCommand delayedcommand;
  bool transfer;
  bool initialized;
  TransferMonitor * tm;
  FileList * fl;
  std::string file;
  int type;
  bool passive;
  bool ssl;
  bool sslclient;
  bool aborted;
  bool transferlocked;
  bool loggedin;
  bool fxp;
  bool listtransfer;
  bool listpassive;
  bool listssl;
  bool listinitialized;
  bool quitting;
  std::string listhost;
  int listport;
  TransferMonitor * listtm;
  std::shared_ptr<CommandOwner> co;
  std::string host;
  int port;
  std::shared_ptr<RecursiveCommandLogic> recursivelogic;
  std::shared_ptr<SiteLogicRequest> request;
  void setTransfer(const std::string &, bool, bool, const std::string &, int, bool, bool);
  void setList(TransferMonitor *, bool, const std::string &, int, bool);
public:
  ConnStateTracker();
  ~ConnStateTracker();
  void delayedCommand(const std::string & command, int delay, bool persisting = false, const std::shared_ptr<CommandOwner> & co = std::shared_ptr<CommandOwner>());
  void timePassed(int);
  int getTimePassed() const;
  void check(const std::shared_ptr<SiteRace> & sr);
  const std::shared_ptr<SiteRace> & lastChecked() const;
  int checkCount() const;
  void purgeSiteRace(const std::shared_ptr<SiteRace> & sr);
  DelayedCommand & getCommand();
  void setDisconnected();
  void setTransfer(const std::string &, bool, bool, bool);
  void setTransfer(const std::string &, bool, const std::string &, int, bool, bool);
  void setList(TransferMonitor *, bool);
  void setList(TransferMonitor *, const std::string &, int, bool);
  bool hasTransfer() const;
  bool hasFileTransfer() const;
  void finishTransfer();
  void finishFileTransfer();
  void abortTransfer();
  bool getTransferAborted() const;
  void lockForTransfer(TransferMonitor *, FileList *, const std::shared_ptr<CommandOwner> &, bool);
  bool isListLocked() const;
  bool isTransferLocked() const;
  bool hasRequest() const;
  bool isLocked() const;
  bool isListOrTransferLocked() const;
  bool isHardLocked() const;
  const std::shared_ptr<SiteLogicRequest> & getRequest() const;
  void setRequest(SiteLogicRequest);
  void finishRequest();
  bool isLoggedIn() const;
  void setLoggedIn();
  void use();
  void resetIdleTime();
  TransferMonitor * getTransferMonitor() const;
  FileList * getTransferFileList() const;
  std::string getTransferFile() const;
  int getTransferType() const;
  bool getTransferPassive() const;
  bool getTransferSSL() const;
  bool getTransferSSLClient() const;
  bool getTransferFXP() const;
  std::string getTransferHost() const;
  int getTransferPort() const;
  const std::shared_ptr<RecursiveCommandLogic> & getRecursiveLogic() const;
  bool transferInitialized() const;
  const std::shared_ptr<CommandOwner> & getCommandOwner() const;
  void initializeTransfer();
  bool isQuitting() const;
  void setQuitting();
};
