#pragma once

#include "core/pointer.h"

class Engine;
class UIBase;
class SiteLogicManager;
class SiteManager;
class TransferManager;
class TickPoke;
class RemoteCommandHandler;
class IOManager;
class WorkManager;
class SkipList;
class EventLog;
class ProxyManager;
class LocalStorage;
class ExternalFileViewing;
class TimeReference;
class SettingsLoaderSaver;

class GlobalContext {
  private:
    Engine * e;
    SettingsLoaderSaver * sls;
    IOManager * iom;
    WorkManager * wm;
    UIBase * uib;
    SiteManager * sm;
    SiteLogicManager * slm;
    TransferManager * tm;
    TickPoke * tp;
    RemoteCommandHandler * rch;
    SkipList * sl;
    Pointer<EventLog> el;
    ProxyManager * pm;
    LocalStorage * ls;
    ExternalFileViewing * efv;
    TimeReference * tr;
  public:
    void linkCore(WorkManager *, TickPoke *, IOManager *, Pointer<EventLog> &);
    void linkComponents(SettingsLoaderSaver *, Engine *,
        UIBase *, SiteManager *, SiteLogicManager *, TransferManager *,
        RemoteCommandHandler *, SkipList *, ProxyManager *,
        LocalStorage *, ExternalFileViewing *, TimeReference *);
    Engine * getEngine() const;
    SettingsLoaderSaver * getSettingsLoaderSaver() const;
    WorkManager * getWorkManager() const;
    IOManager * getIOManager() const;
    UIBase * getUIBase() const;
    SiteManager * getSiteManager() const;
    SiteLogicManager * getSiteLogicManager() const;
    TransferManager * getTransferManager() const;
    TickPoke * getTickPoke() const;
    RemoteCommandHandler * getRemoteCommandHandler() const;
    SkipList * getSkipList() const;
    Pointer<EventLog> & getEventLog();
    ProxyManager * getProxyManager() const;
    LocalStorage * getLocalStorage() const;
    ExternalFileViewing * getExternalFileViewing() const;
    TimeReference * getTimeReference() const;
};

extern GlobalContext * global;
