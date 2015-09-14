#include "globalcontext.h"

#include <vector>
#include <time.h>

#include "lock.h"

static std::vector<Lock> ssllocks;

static void sslLockingCallback(int mode, int n, const char *, int) {
  if (mode & CRYPTO_LOCK) {
    ssllocks[n].lock();
  }
  else {
    ssllocks[n].unlock();
  }
}

static unsigned long sslThreadIdCallback() {
  return (unsigned long) pthread_self();
}

GlobalContext::GlobalContext() {
  init();
}

void GlobalContext::init() {
  ssllocks.resize(CRYPTO_num_locks());
  CRYPTO_set_locking_callback(sslLockingCallback);
  CRYPTO_set_id_callback(sslThreadIdCallback);
  SSL_library_init();
  SSL_load_error_strings();
  ssl_ctx = SSL_CTX_new(SSLv23_client_method());
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(pthread_self(), "cbftp");
#endif
  updateTime();
}

void GlobalContext::linkEventLog(EventLog * el) {
  this->el = el;
}

void GlobalContext::linkWorkManager(WorkManager * wm) {
  this->wm = wm;
}

void GlobalContext::linkTickPoke(TickPoke * tp) {
  this->tp = tp;
}

void GlobalContext::linkComponents(DataFileHandler * dfh, IOManager * iom,
    Engine * e, UIBase * uib, SiteManager * sm, SiteLogicManager * slm,
    TransferManager * tm, RemoteCommandHandler * rch,
    SkipList * sl, ProxyManager * pm, LocalStorage * ls,
    ExternalFileViewing * efv, TimeReference * tr) {
  this->dfh = dfh;
  this->iom = iom;
  this->e = e;
  this->uib = uib;
  this->sm = sm;
  this->slm = slm;
  this->tm = tm;
  this->rch = rch;
  this->sl = sl;
  this->pm = pm;
  this->ls = ls;
  this->efv = efv;
  this->tr = tr;
}

SSL_CTX * GlobalContext::getSSLCTX() const {
  return ssl_ctx;
}

Engine * GlobalContext::getEngine() const {
  return e;
}

DataFileHandler * GlobalContext::getDataFileHandler() const {
  return dfh;
}

IOManager * GlobalContext::getIOManager() const {
  return iom;
}

WorkManager * GlobalContext::getWorkManager() const {
  return wm;
}

UIBase * GlobalContext::getUIBase() const {
  return uib;
}

SiteManager * GlobalContext::getSiteManager() const {
  return sm;
}

SiteLogicManager * GlobalContext::getSiteLogicManager() const {
  return slm;
}

TransferManager * GlobalContext::getTransferManager() const {
  return tm;
}

TickPoke * GlobalContext::getTickPoke() const {
  return tp;
}

RemoteCommandHandler * GlobalContext::getRemoteCommandHandler() const {
  return rch;
}

SkipList * GlobalContext::getSkipList() const {
  return sl;
}

EventLog * GlobalContext::getEventLog() const {
  return el;
}

ProxyManager * GlobalContext::getProxyManager() const {
  return pm;
}

LocalStorage * GlobalContext::getLocalStorage() const {
  return ls;
}

ExternalFileViewing * GlobalContext::getExternalFileViewing() const {
  return efv;
}

TimeReference * GlobalContext::getTimeReference() const {
  return tr;
}

pthread_attr_t * GlobalContext::getPthreadAttr() {
  return &attr;
}

void GlobalContext::updateTime() {
  time_t rawtime;
  time(&rawtime);
  struct tm * timedata = localtime(&rawtime);
  currentyear = timedata->tm_year + 1900;
  currentmonth = timedata->tm_mon + 1;
  currentday = timedata->tm_mday;
}

int GlobalContext::currentYear() const {
  return currentyear;
}

int GlobalContext::currentMonth() const {
  return currentmonth;
}

int GlobalContext::currentDay() const {
  return currentday;
}
