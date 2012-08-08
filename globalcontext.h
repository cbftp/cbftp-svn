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

class Engine;
class UICommunicator;
class SiteThreadManager;
class SiteManager;
class TransferManager;
class DataFileHandler;

class GlobalContext {
  private:
    SSL_CTX * ssl_ctx;
    pthread_attr_t attr;
    Engine * e;
    DataFileHandler * dfh;
    UICommunicator * uic;
    SiteManager * sm;
    SiteThreadManager * stm;
    TransferManager * tm;
    sem_t list_refresh;
    std::string compiletime;
    std::string svnrev;
  public:
    void init();
    GlobalContext();
    void linkComponents(DataFileHandler *, Engine *, UICommunicator *, SiteManager *, SiteThreadManager *, TransferManager *);
    SSL_CTX * getSSLCTX();
    Engine * getEngine();
    DataFileHandler * getDataFileHandler();
    UICommunicator * getUICommunicator();
    SiteManager * getSiteManager();
    SiteThreadManager * getSiteThreadManager();
    TransferManager * getTransferManager();
    pthread_attr_t * getPthreadAttr();
    sem_t * getListRefreshSem();
    static int ctimeMSec();
    std::string ctimeLog();
    std::string getSVNRevision();
    std::string getCompileTime();
    int str2Int(std::string);
    std::string int2Str(int);
    void signal_catch();
    void signal_ignore();

};

extern void sighandler(int);
extern void sighandler_winch(int);
extern void sighandler_ignore(int);
