#include "connstatetracker.h"

#include "delayedcommand.h"
#include "eventlog.h"
#include "recursivecommandlogic.h"
#include "util.h"
#include "sitelogicrequest.h"

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
  recursivelogic(makePointer<RecursiveCommandLogic>()) {
}

ConnStateTracker::~ConnStateTracker() {

}

void ConnStateTracker::delayedCommand(std::string command, int delay) {
  delayedCommand(command, delay, NULL);
}

void ConnStateTracker::delayedCommand(std::string command, int delay, void * arg) {
  delayedCommand(command, delay, arg, false);
}

void ConnStateTracker::delayedCommand(std::string command, int delay, void * arg, bool persisting) {
  delayedcommand.set(command, time + delay, arg, persisting);
}

void ConnStateTracker::timePassed(int time) {
  this->time += time;
  this->idletime += time;
  if (delayedcommand.isActive()) {
    delayedcommand.currentTime(this->time);
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

DelayedCommand & ConnStateTracker::getCommand() {
  return delayedcommand;
}

void ConnStateTracker::setDisconnected() {
  loggedin = false;
  delayedcommand.weakReset();
  request.reset();
  idletime = 0;
}

void ConnStateTracker::use() {
  util::assert(!transferlocked);
  delayedcommand.reset();
  idletime = 0;
}

void ConnStateTracker::resetIdleTime() {
  delayedcommand.reset();
  idletime = 0;
}

void ConnStateTracker::setTransfer(TransferMonitor * tm, std::string path, std::string file, int type, bool fxp, bool passive, std::string addr, bool ssl) {
  util::assert(transferlocked);
  util::assert(!transfer);
  util::assert(!request);
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
  util::assert(!transferlocked);
  util::assert(!listtransfer);
  util::assert(!transfer);
  use();
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
  util::assert(!transferlocked);
  util::assert(!transfer);
  util::assert(!request);
  use();
  transferlocked = true;
  lockeddownload = download;
}

bool ConnStateTracker::isLocked() const {
  return isHardLocked() || isListLocked();
}

bool ConnStateTracker::isListLocked() const {
  return listtransfer;
}

bool ConnStateTracker::isHardLocked() const {
  return transferlocked || hasRequest();
}

bool ConnStateTracker::isLockedForDownload() const {
  return transferlocked && lockeddownload;
}

bool ConnStateTracker::isLockedForUpload() const {
  return transferlocked && !lockeddownload;
}

bool ConnStateTracker::hasRequest() const {
  return !!request;
}

const Pointer<SiteLogicRequest> & ConnStateTracker::getRequest() const {
  return request;
}

void ConnStateTracker::setRequest(SiteLogicRequest request) {
  this->request = makePointer<SiteLogicRequest>(request);
}

void ConnStateTracker::finishRequest() {
  request.reset();
}

Pointer<RecursiveCommandLogic> ConnStateTracker::getRecursiveLogic() const {
  return recursivelogic;
}

bool ConnStateTracker::transferInitialized() const {
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
