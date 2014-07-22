#pragma once

#include <string>
#include <list>

#define CST_DOWNLOAD 981
#define CST_UPLOAD 982
#define CST_LIST 983

class SiteRace;
class FileList;
class TransferMonitor;
class DelayedCommand;
class RecursiveCommandLogic;

class ConnStateTracker {
private:
  int time;
  int idletime;
  int lastcheckedcount;
  SiteRace * lastchecked;
  std::list<DelayedCommand> releasedcommands;
  std::list<DelayedCommand> delayedcommands;
  bool transfer;
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
  std::string listaddr;
  TransferMonitor * listtm;
  std::string addr;
  RecursiveCommandLogic * recursivelogic;
  void setTransfer(TransferMonitor *, std::string, std::string, int, bool, bool, std::string, bool);
  void setList(TransferMonitor *, bool, std::string, bool);
public:
  ConnStateTracker();
  void delayedCommand(std::string, int);
  void delayedCommand(std::string, int, void *);
  void timePassed(int);
  int getTimePassed();
  void check(SiteRace *);
  SiteRace * lastChecked();
  int checkCount();
  bool hasReleasedCommand();
  DelayedCommand getCommand();
  void setDisconnected();
  void setTransfer(TransferMonitor *, std::string, std::string, int, bool, bool);
  void setTransfer(TransferMonitor *, std::string, std::string, int, std::string, bool);
  void setList(TransferMonitor *, bool);
  void setList(TransferMonitor *, std::string, bool);
  bool hasTransfer();
  void finishTransfer();
  void abortTransfer();
  bool getTransferAborted();
  void lockForTransfer(bool);
  bool isLocked();
  bool isLockedForDownload();
  bool isLockedForUpload();
  bool isLoggedIn();
  void setLoggedIn();
  void use();
  void resetIdleTime();
  TransferMonitor * getTransferMonitor();
  std::string getTransferPath();
  std::string getTransferFile();
  int getTransferType();
  bool getTransferPassive();
  bool getTransferSSL();
  bool getTransferFXP();
  std::string getTransferAddr();
  RecursiveCommandLogic * getRecursiveLogic();
};
