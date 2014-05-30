#pragma once

#include <openssl/ssl.h>
#include <pthread.h>
#include <string>

class Engine;
class UIBase;
class SiteLogicManager;
class SiteManager;
class TransferManager;
class DataFileHandler;
class TickPoke;
class RemoteCommandHandler;
class IOManager;
class WorkManager;
class SkipList;
class EventLog;
class ProxyManager;
class LocalStorage;
class ExternalFileViewing;

class GlobalContext {
  private:
    SSL_CTX * ssl_ctx;
    pthread_attr_t attr;
    Engine * e;
    DataFileHandler * dfh;
    IOManager * iom;
    WorkManager * wm;
    UIBase * uib;
    SiteManager * sm;
    SiteLogicManager * slm;
    TransferManager * tm;
    TickPoke * tp;
    RemoteCommandHandler * rch;
    SkipList * sl;
    EventLog * el;
    ProxyManager * pm;
    LocalStorage * ls;
    ExternalFileViewing * efv;
    pthread_mutex_t time_mutex;
    std::string compiletime;
    std::string svnrev;
    int currentyear;
    int currentmonth;
    int currentday;
  public:
    void init();
    GlobalContext();
    void linkEventLog(EventLog *);
    void linkWorkManager(WorkManager *);
    void linkComponents(DataFileHandler *, IOManager *, Engine *,
        UIBase *, SiteManager *, SiteLogicManager *, TransferManager *,
        TickPoke *, RemoteCommandHandler *, SkipList *, ProxyManager *,
        LocalStorage *, ExternalFileViewing *);
    SSL_CTX * getSSLCTX();
    Engine * getEngine();
    DataFileHandler * getDataFileHandler();
    WorkManager * getWorkManager();
    IOManager * getIOManager();
    UIBase * getUIBase();
    SiteManager * getSiteManager();
    SiteLogicManager * getSiteLogicManager();
    TransferManager * getTransferManager();
    TickPoke * getTickPoke();
    RemoteCommandHandler * getRemoteCommandHandler();
    SkipList * getSkipList();
    EventLog * getEventLog();
    ProxyManager * getProxyManager();
    LocalStorage * getLocalStorage();
    ExternalFileViewing * getExternalFileViewing();
    pthread_attr_t * getPthreadAttr();
    static int ctimeMSec();
    void updateTime();
    int currentYear();
    int currentMonth();
    int currentDay();
    std::string ctimeLog();
    std::string getSVNRevision();
    std::string getCompileTime();
    int str2Int(std::string);
    std::string int2Str(int);
    std::string int2Str(unsigned int);
    std::string int2Str(unsigned long long int);
    void signal_catch();
    void signal_ignore();
    std::string & debugString(const char *);
};

extern void sighandler(int);
extern void sighandler_winch(int);
extern void sighandler_ignore(int);
