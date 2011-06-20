#pragma once
#include <signal.h>
#include <iostream>
#include <openssl/ssl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/timeb.h>
#include <string>
#include <sstream>

class Engine;
class UserInterface;
class SiteThreadManager;
class SiteManager;
class TransferManager;

class GlobalContext {
  private:
    SSL_CTX * ssl_ctx;
    pthread_attr_t attr;
    Engine * e;
    UserInterface * ui;
    SiteManager * sm;
    SiteThreadManager * stm;
    TransferManager * tm;
    sem_t list_refresh;
    std::string compiletime;
    std::string svnrev;
  public:
    void init();
    GlobalContext();
    void linkManagers(UserInterface *, SiteManager *, SiteThreadManager *, TransferManager *);
    void linkEngine(Engine *);
    SSL_CTX * getSSLCTX();
    Engine * getEngine();
    UserInterface * getUI();
    SiteManager * getSiteManager();
    SiteThreadManager * getSiteThreadManager();
    TransferManager * getTransferManager();
    pthread_attr_t * getPthreadAttr();
    sem_t * getListRefreshSem();
    int ctimeMSec();
    std::string getSVNRevision();
    std::string getCompileTime();
    int str2Int(std::string);
    std::string int2Str(int);
    void signal_catch();
    void signal_ignore();

};

extern void sighandler(int);
extern void sighandler_ignore(int);
