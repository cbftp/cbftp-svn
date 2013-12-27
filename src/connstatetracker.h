#pragma once

#include <string>
#include <list>

class SiteRace;
class FileList;
class TransferMonitor;
class DelayedCommand;
class RecursiveCommandLogic;

class ConnStateTracker {
private:
  int state;
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
  bool download;
  bool passive;
  bool ssl;
  bool aborted;
  bool transferlocked;
  bool lockeddownload;
  bool loggedin;
  bool fxp;
  std::string addr;
  RecursiveCommandLogic * recursivelogic;
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
  void setIdle();
  void setReady();
  void setBusy();
  bool isDisconnected();
  bool isIdle();
  bool isReady();
  void setTransfer(TransferMonitor *, std::string, std::string, bool, bool, bool);
  void setTransfer(TransferMonitor *, std::string, std::string, bool, std::string, bool);
  bool hasTransfer();
  void finishTransfer();
  void abortTransfer();
  bool getTransferAborted();
  void lockForTransfer(bool);
  bool isLockedForDownload();
  bool isLockedForUpload();
  bool isLoggedIn();
  void setLoggedIn();
  TransferMonitor * getTransferMonitor();
  std::string getTransferPath();
  std::string getTransferFile();
  bool getTransferDownload();
  bool getTransferPassive();
  bool getTransferSSL();
  bool getTransferFXP();
  std::string getTransferAddr();
  RecursiveCommandLogic * getRecursiveLogic();
};
