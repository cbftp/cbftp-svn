#pragma once

#include <string>
#include <list>

#include "delayedcommand.h"
#include "pointer.h"

#define CST_DOWNLOAD 981
#define CST_UPLOAD 982
#define CST_LIST 983

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
  std::string path;
  std::string file;
  int type;
  bool passive;
  bool ssl;
  bool aborted;
  bool transferlocked;
  bool lockeddownload;
  bool loggedin;
  bool fxp;
  bool listtransfer;
  bool listpassive;
  bool listssl;
  bool listinitialized;
  std::string listaddr;
  TransferMonitor * listtm;
  std::string addr;
  Pointer<RecursiveCommandLogic> recursivelogic;
  Pointer<SiteLogicRequest> request;
  void setTransfer(TransferMonitor *, std::string, std::string, int, bool, bool, std::string, bool);
  void setList(TransferMonitor *, bool, std::string, bool);
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
  void setTransfer(TransferMonitor *, std::string, std::string, int, bool, bool);
  void setTransfer(TransferMonitor *, std::string, std::string, int, std::string, bool);
  void setList(TransferMonitor *, bool);
  void setList(TransferMonitor *, std::string, bool);
  bool hasTransfer() const;
  bool hasFileTransfer() const;
  void finishTransfer();
  void abortTransfer();
  bool getTransferAborted() const;
  void lockForTransfer(bool);
  bool isLocked() const;
  bool isListLocked() const;
  bool isHardLocked() const;
  bool isLockedForDownload() const;
  bool isLockedForUpload() const;
  bool hasRequest() const;
  const Pointer<SiteLogicRequest> & getRequest() const;
  void setRequest(SiteLogicRequest);
  void finishRequest();
  bool isLoggedIn() const;
  void setLoggedIn();
  void use();
  void resetIdleTime();
  TransferMonitor * getTransferMonitor() const;
  std::string getTransferPath() const;
  std::string getTransferFile() const;
  int getTransferType() const;
  bool getTransferPassive() const;
  bool getTransferSSL() const;
  bool getTransferFXP() const;
  std::string getTransferAddr() const;
  Pointer<RecursiveCommandLogic> getRecursiveLogic() const;
  bool transferInitialized() const;
  void initializeTransfer();
};
