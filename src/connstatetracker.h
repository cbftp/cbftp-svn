#pragma once

#include <string>
#include <list>

#include "delayedcommand.h"

class SiteRace;
class FileList;
class TransferMonitorBase;

class ConnStateTracker {
private:
  int state;
  int time;
  int lastcheckedcount;
  SiteRace * lastchecked;
  std::list<DelayedCommand> releasedcommands;
  std::list<DelayedCommand> delayedcommands;
  bool transfer;
  TransferMonitorBase * tmb;
  FileList * fls;
  std::string file;
  bool download;
  bool passive;
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
  void setTransfer(TransferMonitorBase *, FileList *, std::string, bool, bool);
  void setTransfer(TransferMonitorBase *, FileList *, std::string, bool, bool, std::string);
  bool hasTransfer();
  void finishTransfer();
  TransferMonitorBase * getTransferMonitor();
  FileList * getTransferFileList();
  std::string getTransferFile();
  bool getTransferDownload();
  bool getTransferPassive();
  std::string getTransferAddr();
};
