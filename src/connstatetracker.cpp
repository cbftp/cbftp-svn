#include "connstatetracker.h"

#include "delayedcommand.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "recursivecommandlogic.h"

extern GlobalContext * global;

ConnStateTracker::ConnStateTracker() :
  time(0),
  idletime(0),
  lastcheckedcount(0),
  lastchecked(NULL),
  transfer(false),
  initialized(false),
  tm(NULL),
  aborted(false),
  transferlocked(false),
  lockeddownload(false),
  loggedin(false),
  fxp(false),
  listtransfer(false),
  listinitialized(false),
  recursivelogic(new RecursiveCommandLogic()) {
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

int ConnStateTracker::getTimePassed() const {
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

SiteRace * ConnStateTracker::lastChecked() const {
  return lastchecked;
}

int ConnStateTracker::checkCount() const {
    return lastcheckedcount;
}

bool ConnStateTracker::hasReleasedCommand() const {
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
  this->initialized = false;
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
  if (this->transferlocked) {
    global->getEventLog()->log("ConnStateTracker", "BUG: Setting list while being transfer locked!");
  }
  if (this->listtransfer) {
    global->getEventLog()->log("ConnStateTracker", "BUG: Setting list while already having a list!");
  }
  if (this->transfer) {
    global->getEventLog()->log("ConnStateTracker", "BUG: Setting list while already having a transfer!");
  }
  this->listtransfer = true;
  this->listinitialized = false;
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

bool ConnStateTracker::isLoggedIn() const {
  return loggedin;
}

void ConnStateTracker::setLoggedIn() {
  loggedin = true;
}

bool ConnStateTracker::hasTransfer() const {
  return isListLocked() || hasFileTransfer();
}

bool ConnStateTracker::hasFileTransfer() const {
  return transfer;
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

bool ConnStateTracker::getTransferAborted() const {
  return aborted;
}

TransferMonitor * ConnStateTracker::getTransferMonitor() const {
  if (listtransfer) {
    return listtm;
  }
  if (transfer) {
    return tm;
  }
  return NULL;
}

std::string ConnStateTracker::getTransferPath() const {
  return path;
}

std::string ConnStateTracker::getTransferFile() const {
  return file;
}

int ConnStateTracker::getTransferType() const {
  if (listtransfer) {
    return CST_LIST;
  }
  return type;
}

bool ConnStateTracker::getTransferPassive() const {
  if (listtransfer) {
    return listpassive;
  }
  return passive;
}

bool ConnStateTracker::getTransferFXP() const {
  if (listtransfer) {
    return false;
  }
  return fxp;
}

std::string ConnStateTracker::getTransferAddr() const {
  if (listtransfer) {
    return listaddr;
  }
  return addr;
}

bool ConnStateTracker::getTransferSSL() const{
  if (listtransfer) {
    return listssl;
  }
  return ssl;
}

void ConnStateTracker::lockForTransfer(bool download) {
  transferlocked = true;
  lockeddownload = download;
}

bool ConnStateTracker::isLocked() const {
  return isTransferLocked() || isListLocked();
}

bool ConnStateTracker::isListLocked() const {
  return listtransfer;
}

bool ConnStateTracker::isTransferLocked() const {
  return transferlocked;
}

bool ConnStateTracker::isLockedForDownload() const {
  return isTransferLocked() && lockeddownload;
}

bool ConnStateTracker::isLockedForUpload() const {
  return isTransferLocked() && !lockeddownload;
}

RecursiveCommandLogic * ConnStateTracker::getRecursiveLogic() const {
  return recursivelogic;
}

bool ConnStateTracker::transferInitialized() {
  if (listtransfer) {
    return listinitialized;
  }
  return transfer && initialized;
}

void ConnStateTracker::initializeTransfer() {
  if (listtransfer) {
    listinitialized = true;
  }
  else {
    initialized = true;
  }
}
