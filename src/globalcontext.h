#pragma once

#include <openssl/ssl.h>
#include <pthread.h>
#include <string>
#include <vector>

#define SIZEPOWER 1024
#define SIZEDECIMALS 2

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
    void linkTickPoke(TickPoke *);
    void linkComponents(DataFileHandler *, IOManager *, Engine *,
        UIBase *, SiteManager *, SiteLogicManager *, TransferManager *,
        RemoteCommandHandler *, SkipList *, ProxyManager *,
        LocalStorage *, ExternalFileViewing *);
    SSL_CTX * getSSLCTX() const;
    Engine * getEngine() const;
    DataFileHandler * getDataFileHandler() const;
    WorkManager * getWorkManager() const;
    IOManager * getIOManager() const;
    UIBase * getUIBase() const;
    SiteManager * getSiteManager() const;
    SiteLogicManager * getSiteLogicManager() const;
    TransferManager * getTransferManager() const;
    TickPoke * getTickPoke() const;
    RemoteCommandHandler * getRemoteCommandHandler() const;
    SkipList * getSkipList() const;
    EventLog * getEventLog() const;
    ProxyManager * getProxyManager() const;
    LocalStorage * getLocalStorage() const;
    ExternalFileViewing * getExternalFileViewing() const;
    pthread_attr_t * getPthreadAttr();
    static int ctimeMSec();
    static std::string parseSize(unsigned long long int);
    static int getSizeGranularity();
    static std::vector<unsigned long long int> getPowers();
    void updateTime();
    int currentYear() const;
    int currentMonth() const;
    int currentDay() const;
    static std::string ctimeLog();
    std::string getSVNRevision() const;
    std::string getCompileTime() const;
    static int str2Int(std::string);
    static std::string int2Str(int);
    static std::string int2Str(unsigned int);
    static std::string int2Str(unsigned long long int);
    void signal_catch();
    void signal_ignore();
    std::string & debugString(const char *);
};

extern void sighandler(int);
extern void sighandler_winch(int);
extern void sighandler_ignore(int);
