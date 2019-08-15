#pragma once

#include <memory>

namespace Core {
class IOManager;
class TickPoke;
class WorkManager;
}

class Engine;
class UIBase;
class SiteLogicManager;
class SiteManager;
class TransferManager;
class RemoteCommandHandler;
class SkipList;
class EventLog;
class ProxyManager;
class LocalStorage;
class ExternalFileViewing;
class TimeReference;
class SettingsLoaderSaver;
class Statistics;
class SectionManager;

class GlobalContext {
  private:
    Engine * e;
    SettingsLoaderSaver * sls;
    Core::IOManager * iom;
    Core::WorkManager * wm;
    UIBase * uib;
    SiteManager * sm;
    SiteLogicManager * slm;
    TransferManager * tm;
    Core::TickPoke * tp;
    RemoteCommandHandler * rch;
    SkipList * sl;
    std::shared_ptr<EventLog> el;
    ProxyManager * pm;
    LocalStorage * ls;
    ExternalFileViewing * efv;
    TimeReference * tr;
    Statistics * s;
    SectionManager * secm;
  public:
    void linkCore(Core::WorkManager*, Core::TickPoke*, Core::IOManager*, std::shared_ptr<EventLog>&);
    void linkComponents(SettingsLoaderSaver *, Engine *,
        UIBase *, SiteManager *, SiteLogicManager *, TransferManager *,
        RemoteCommandHandler *, SkipList *, ProxyManager *,
        LocalStorage *, ExternalFileViewing *, TimeReference *, Statistics *, SectionManager *);
    Engine * getEngine() const;
    SettingsLoaderSaver * getSettingsLoaderSaver() const;
    Core::WorkManager * getWorkManager() const;
    Core::IOManager * getIOManager() const;
    UIBase * getUIBase() const;
    SiteManager * getSiteManager() const;
    SiteLogicManager * getSiteLogicManager() const;
    TransferManager * getTransferManager() const;
    Core::TickPoke * getTickPoke() const;
    RemoteCommandHandler * getRemoteCommandHandler() const;
    SkipList * getSkipList() const;
    std::shared_ptr<EventLog> & getEventLog();
    ProxyManager * getProxyManager() const;
    LocalStorage * getLocalStorage() const;
    ExternalFileViewing * getExternalFileViewing() const;
    TimeReference * getTimeReference() const;
    Statistics * getStatistics() const;
    SectionManager * getSectionManager() const;
};

extern GlobalContext * global;
