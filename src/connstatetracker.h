#pragma once

#include <string>
#include <list>

class SiteRace;
class FileList;
class TransferMonitor;
class DelayedCommand;

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
  FileList * fls;
  std::string file;
  bool download;
  bool passive;
  bool ssl;
  bool aborted;
  bool transferlocked;
  bool lockeddownload;
  bool loggedin;
  std::string addr;
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
  void setTransfer(TransferMonitor *, FileList *, std::string, bool, bool, bool);
  void setTransfer(TransferMonitor *, FileList *, std::string, bool, bool, std::string, bool);
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
  FileList * getTransferFileList();
  std::string getTransferFile();
  bool getTransferDownload();
  bool getTransferPassive();
  bool getTransferSSL();
  std::string getTransferAddr();
};
