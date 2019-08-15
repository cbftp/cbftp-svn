#include "globalcontext.h"

GlobalContext * global = new GlobalContext();

void GlobalContext::linkCore(Core::WorkManager * wm, Core::TickPoke * tp, Core::IOManager * iom, std::shared_ptr<EventLog> & el) {
  this->wm = wm;
  this->tp = tp;
  this->iom = iom;
  this->el = el;
}

void GlobalContext::linkComponents(SettingsLoaderSaver * sls, Engine * e,
    UIBase * uib, SiteManager * sm, SiteLogicManager * slm,
    TransferManager * tm, RemoteCommandHandler * rch,
    SkipList * sl, ProxyManager * pm, LocalStorage * ls,
    ExternalFileViewing * efv, TimeReference * tr, Statistics * s, SectionManager * secm) {
  this->sls = sls;
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
  this->s = s;
  this->secm = secm;
}

Engine * GlobalContext::getEngine() const {
  return e;
}

SettingsLoaderSaver * GlobalContext::getSettingsLoaderSaver() const {
  return sls;
}

Core::IOManager * GlobalContext::getIOManager() const {
  return iom;
}

Core::WorkManager * GlobalContext::getWorkManager() const {
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

Core::TickPoke * GlobalContext::getTickPoke() const {
  return tp;
}

RemoteCommandHandler * GlobalContext::getRemoteCommandHandler() const {
  return rch;
}

SkipList * GlobalContext::getSkipList() const {
  return sl;
}

std::shared_ptr<EventLog> & GlobalContext::getEventLog() {
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

Statistics * GlobalContext::getStatistics() const {
  return s;
}

SectionManager * GlobalContext::getSectionManager() const {
  return secm;
}

