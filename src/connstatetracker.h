#pragma once

#include <string>
#include <list>

#include "delayedcommand.h"
#include "core/pointer.h"

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
  SiteRace * lastchecked;
  DelayedCommand delayedcommand;
  bool transfer;
  bool initialized;
  TransferMonitor * tm;
  FileList * fl;
  FileList * listfl;
  std::string file;
  int type;
  bool passive;
  bool ssl;
  bool aborted;
  bool transferlocked;
  bool loggedin;
  bool fxp;
  bool listtransfer;
  bool listpassive;
  bool listssl;
  bool listinitialized;
  std::string listhost;
  int listport;
  TransferMonitor * listtm;
  std::string host;
  int port;
  Pointer<RecursiveCommandLogic> recursivelogic;
  Pointer<SiteLogicRequest> request;
  CommandOwner * listcommandowner;
  void setTransfer(const std::string &, bool, bool, const std::string &, int, bool);
  void setList(TransferMonitor *, bool, const std::string &, int, bool, CommandOwner *, FileList *);
public:
  ConnStateTracker();
  ~ConnStateTracker();
  void delayedCommand(std::string, int);
  void delayedCommand(std::string, int, void *);
  void delayedCommand(std::string, int, void *, bool);
  void timePassed(int);
  int getTimePassed() const;
  void check(SiteRace *);
  SiteRace * lastChecked() const;
  int checkCount() const;
  DelayedCommand & getCommand();
  void setDisconnected();
  void setTransfer(const std::string &, bool, bool);
  void setTransfer(const std::string &, const std::string &, int, bool);
  void setList(TransferMonitor *, bool, CommandOwner *, FileList *);
  void setList(TransferMonitor *, const std::string &, int, bool, CommandOwner *, FileList *);
  bool hasTransfer() const;
  bool hasFileTransfer() const;
  void finishTransfer();
  void abortTransfer();
  bool getTransferAborted() const;
  void lockForTransfer(TransferMonitor *, FileList *, bool);
  bool isListLocked() const;
  bool isTransferLocked() const;
  bool hasRequest() const;
  bool isLocked() const;
  bool isListOrTransferLocked() const;
  bool isHardLocked() const;
  const Pointer<SiteLogicRequest> & getRequest() const;
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
  bool getTransferFXP() const;
  std::string getTransferHost() const;
  int getTransferPort() const;
  Pointer<RecursiveCommandLogic> getRecursiveLogic() const;
  bool transferInitialized() const;
  void initializeTransfer();
  CommandOwner * getListCommandOwner() const;
  FileList * getListFileList() const;
};
