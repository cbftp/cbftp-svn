#include "connstatetracker.h"

#include "delayedcommand.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "recursivecommandlogic.h"

extern GlobalContext * global;

ConnStateTracker::ConnStateTracker() {
  time = 0;
  idletime = 0;
  lastchecked = NULL;
  lastcheckedcount = 0;
  transfer = false;
  transferlocked = false;
  lockeddownload = false;
  aborted = false;
  loggedin = false;
  fxp = false;
  listtransfer = false;
  recursivelogic = new RecursiveCommandLogic();
}
void ConnStateTracker::delayedCommand(std::string command, int delay) {
  delayedcommands.push_back(DelayedCommand(command, delay + time));
}

void ConnStateTracker::delayedCommand(std::string command, int delay, void * arg) {
  delayedcommands.push_back(DelayedCommand(command, delay + time, arg));
}

void ConnStateTracker::timePassed(int time) {
  this->time += time;
  this->idletime += time;
  std::list<DelayedCommand>::iterator it;
  bool found;
  while (true) {
    found = false;
    for (it = delayedcommands.begin(); it != delayedcommands.end(); it++) {
      if (this->time >= it->getDelay()) {
        releasedcommands.push_back(*it);
        delayedcommands.erase(it);
        found = true;
        break;
      }
    }
    if (!found) {
      break;
    }
  }
}

int ConnStateTracker::getTimePassed() {
  return idletime;
}

void ConnStateTracker::check(SiteRace * sr) {
  if (lastchecked == sr) {
    lastcheckedcount++;
  }
  else {
    lastchecked = sr;
    lastcheckedcount = 1;
  }
}

SiteRace * ConnStateTracker::lastChecked() {
  return lastchecked;
}

int ConnStateTracker::checkCount() {
    return lastcheckedcount;
}

bool ConnStateTracker::hasReleasedCommand() {
  return releasedcommands.size() > 0;
}

DelayedCommand ConnStateTracker::getCommand() {
  if (releasedcommands.size() > 0) {
    DelayedCommand command = releasedcommands.front();
    releasedcommands.pop_front();
    return command;
  }
  //undefined behavior
  return DelayedCommand("error", 0);
}

void ConnStateTracker::setDisconnected() {
  loggedin = false;
  delayedcommands.clear();
  idletime = 0;
}

void ConnStateTracker::use() {
  if (transferlocked) {
    *(int*)0=0; // crash on purpose
  }
  delayedcommands.clear();
  idletime = 0;
}

void ConnStateTracker::resetIdleTime() {
  idletime = 0;
}

void ConnStateTracker::setTransfer(TransferMonitor * tm, std::string path, std::string file, int type, bool fxp, bool passive, std::string addr, bool ssl) {
  if (this->transfer) {
    global->getEventLog()->log("ConnStateTracker", "BUG: Setting transfer while already having a transfer!");
  }
  this->transfer = true;
  this->aborted = false;
  this->tm = tm;
  this->path = path;
  this->file = file;
  this->type = type;
  this->fxp = fxp;
  this->passive = passive;
  this->addr = addr;
  this->ssl = ssl;
}

void ConnStateTracker::setTransfer(TransferMonitor * tm, std::string path, std::string file, int type, bool fxp, bool ssl) {
  setTransfer(tm, path, file, type, fxp, true, "", ssl);
}

void ConnStateTracker::setTransfer(TransferMonitor * tm, std::string path, std::string file, int type, std::string addr, bool ssl) {
  setTransfer(tm, path, file, type, false, false, addr, ssl);
}

void ConnStateTracker::setList(TransferMonitor * tm, bool listpassive, std::string addr, bool ssl) {
  if (this->listtransfer) {
    global->getEventLog()->log("ConnStateTracker", "BUG: Setting list while already having a list!");
  }
  if (this->transfer) {
    global->getEventLog()->log("ConnStateTracker", "BUG: Setting list while already having a transfer!");
  }
  this->listtransfer = true;
  this->listtm = tm;
  this->listpassive = listpassive;
  this->listaddr = addr;
  this->listssl = ssl;
}

void ConnStateTracker::setList(TransferMonitor * tm, bool ssl) {
  setList(tm, true, "", ssl);
}

void ConnStateTracker::setList(TransferMonitor * tm, std::string addr, bool ssl) {
  setList(tm, false, addr, ssl);
}

bool ConnStateTracker::isLoggedIn() {
  return loggedin;
}

void ConnStateTracker::setLoggedIn() {
  loggedin = true;
}

bool ConnStateTracker::hasTransfer() {
  return transfer || listtransfer;
}

void ConnStateTracker::finishTransfer() {
  if (listtransfer) {
    listtransfer = false;
    return;
  }
  transfer = false;
  transferlocked = false;
}

void ConnStateTracker::abortTransfer() {
  aborted = true;
}

bool ConnStateTracker::getTransferAborted() {
  return aborted;
}

TransferMonitor * ConnStateTracker::getTransferMonitor() {
  if (listtransfer) {
    return listtm;
  }
  if (transfer) {
    return tm;
  }
  return NULL;
}

std::string ConnStateTracker::getTransferPath() {
  return path;
}

std::string ConnStateTracker::getTransferFile() {
  return file;
}

int ConnStateTracker::getTransferType() {
  if (listtransfer) {
    return CST_LIST;
  }
  return type;
}

bool ConnStateTracker::getTransferPassive() {
  if (listtransfer) {
    return listpassive;
  }
  return passive;
}

bool ConnStateTracker::getTransferFXP() {
  if (listtransfer) {
    return false;
  }
  return fxp;
}

std::string ConnStateTracker::getTransferAddr() {
  if (listtransfer) {
    return listaddr;
  }
  return addr;
}

bool ConnStateTracker::getTransferSSL() {
  if (listtransfer) {
    return listssl;
  }
  return ssl;
}

void ConnStateTracker::lockForTransfer(bool download) {
  transferlocked = true;
  lockeddownload = download;
}

bool ConnStateTracker::isLocked() {
  return transferlocked;
}

bool ConnStateTracker::isLockedForDownload() {
  return transferlocked && lockeddownload;
}

bool ConnStateTracker::isLockedForUpload() {
  return transferlocked && !lockeddownload;
}

RecursiveCommandLogic * ConnStateTracker::getRecursiveLogic() {
  return recursivelogic;
}
