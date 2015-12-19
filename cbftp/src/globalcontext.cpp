#include "globalcontext.h"

#include <time.h>

void GlobalContext::linkEventLog(EventLog * el) {
  this->el = el;
}

void GlobalContext::linkWorkManager(WorkManager * wm) {
  this->wm = wm;
}

void GlobalContext::linkTickPoke(TickPoke * tp) {
  this->tp = tp;
}

void GlobalContext::linkComponents(SettingsLoaderSaver * sls, IOManager * iom,
    Engine * e, UIBase * uib, SiteManager * sm, SiteLogicManager * slm,
    TransferManager * tm, RemoteCommandHandler * rch,
    SkipList * sl, ProxyManager * pm, LocalStorage * ls,
    ExternalFileViewing * efv, TimeReference * tr) {
  this->sls = sls;
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

Engine * GlobalContext::getEngine() const {
  return e;
}

SettingsLoaderSaver * GlobalContext::getSettingsLoaderSaver() const {
  return sls;
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
