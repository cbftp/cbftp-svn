#pragma once

#include <signal.h>
#include <iostream>
#include <openssl/ssl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/timeb.h>
#include <time.h>
#include <string>
#include <sstream>
#include <ctime>

class Engine;
class UICommunicator;
class SiteThreadManager;
class SiteManager;
class TransferManager;
class DataFileHandler;
class TickPoke;
class RemoteCommandHandler;
class IOManager;
class WorkManager;

class GlobalContext {
  private:
    SSL_CTX * ssl_ctx;
    pthread_attr_t attr;
    Engine * e;
    DataFileHandler * dfh;
    IOManager * iom;
    WorkManager * wm;
    UICommunicator * uic;
    SiteManager * sm;
    SiteThreadManager * stm;
    TransferManager * tm;
    TickPoke * tp;
    RemoteCommandHandler * rch;
    sem_t list_refresh;
    pthread_mutex_t time_mutex;
    std::string compiletime;
    std::string svnrev;
    int currentyear;
    int currentmonth;
    int currentday;
  public:
    void init();
    GlobalContext();
    void linkWorkManager(WorkManager *);
    void linkComponents(DataFileHandler *, IOManager *, Engine *, UICommunicator *, SiteManager *, SiteThreadManager *, TransferManager *, TickPoke *, RemoteCommandHandler *);
    SSL_CTX * getSSLCTX();
    Engine * getEngine();
    DataFileHandler * getDataFileHandler();
    WorkManager * getWorkManager();
    IOManager * getIOManager();
    UICommunicator * getUICommunicator();
    SiteManager * getSiteManager();
    SiteThreadManager * getSiteThreadManager();
    TransferManager * getTransferManager();
    TickPoke * getTickPoke();
    RemoteCommandHandler * getRemoteCommandHandler();
    pthread_attr_t * getPthreadAttr();
    sem_t * getListRefreshSem();
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

};

extern void sighandler(int);
extern void sighandler_winch(int);
extern void sighandler_ignore(int);
