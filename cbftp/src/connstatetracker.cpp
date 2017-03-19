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
  util::assert(!transferlocked);
  util::assert(!listtransfer);
  util::assert(!transfer);
  util::assert(!request);
  loggedin = false;
  delayedcommand.weakReset();
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

void ConnStateTracker::setTransfer(const std::string & file, bool fxp, bool passive, const std::string & host, int port, bool ssl) {
  util::assert(transferlocked);
  util::assert(!transfer);
  util::assert(!request);
  util::assert(tm);
  this->transfer = true;
  this->initialized = false;
  this->file = file;
  this->fxp = fxp;
  this->passive = passive;
  this->host = host;
  this->port = port;
  this->ssl = ssl;
}

void ConnStateTracker::setTransfer(const std::string & file, bool fxp, bool ssl) {
  setTransfer(file, fxp, true, "", 0, ssl);
}

void ConnStateTracker::setTransfer(const std::string & file, const std::string & host, int port, bool ssl) {
  setTransfer(file, false, false, host, port, ssl);
}

void ConnStateTracker::setList(TransferMonitor * tm, bool listpassive, const std::string & host, int port, bool ssl) {
  util::assert(!transferlocked);
  util::assert(!listtransfer);
  util::assert(!transfer);
  use();
  this->listtransfer = true;
  this->listinitialized = false;
  this->listtm = tm;
  this->listpassive = listpassive;
  this->listhost = host;
  this->listport = port;
  this->listssl = ssl;
}

void ConnStateTracker::setList(TransferMonitor * tm, bool ssl) {
  setList(tm, true, "", 0, ssl);
}

void ConnStateTracker::setList(TransferMonitor * tm, const std::string & host, int port, bool ssl) {
  setList(tm, false, host, port, ssl);
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
  tm = NULL;
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
  if (transferlocked) {
    return tm;
  }
  return NULL;
}

FileList * ConnStateTracker::getTransferFileList() const {
  return fl;
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

std::string ConnStateTracker::getTransferHost() const {
  if (listtransfer) {
    return listhost;
  }
  return host;
}

int ConnStateTracker::getTransferPort() const {
  if (listtransfer) {
    return listport;
  }
  return port;
}

bool ConnStateTracker::getTransferSSL() const{
  if (listtransfer) {
    return listssl;
  }
  return ssl;
}

void ConnStateTracker::lockForTransfer(TransferMonitor * tm, FileList * fl, bool download) {
  util::assert(!transferlocked);
  util::assert(!transfer);
  util::assert(!request);
  use();
  this->tm = tm;
  this->fl = fl;
  aborted = false;
  transferlocked = true;
  type = download ? CST_DOWNLOAD : CST_UPLOAD;
}

bool ConnStateTracker::isListLocked() const {
  return listtransfer;
}

bool ConnStateTracker::isTransferLocked() const {
  return transferlocked;
}

bool ConnStateTracker::hasRequest() const {
  return !!request;
}

bool ConnStateTracker::isLocked() const {
  return isListOrTransferLocked() || hasRequest();
}

bool ConnStateTracker::isListOrTransferLocked() const {
  return isListLocked() || isTransferLocked();
}

bool ConnStateTracker::isHardLocked() const {
  return isTransferLocked() || hasRequest();
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
