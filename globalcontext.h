#pragma once
#include <openssl/ssl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/timeb.h>

class SiteThreadManager;
class SiteManager;
class TransferManager;

class GlobalContext {
  private:
    SSL_CTX * ssl_ctx;
    pthread_attr_t attr;
    SiteManager * sm;
    SiteThreadManager * stm;
    TransferManager * tm;
    sem_t list_refresh;
  public:
    void init();
    GlobalContext();
    void linkManagers(SiteManager *, SiteThreadManager *, TransferManager *);
    SSL_CTX * getSSLCTX();
    SiteManager * getSiteManager();
    SiteThreadManager * getSiteThreadManager();
    TransferManager * getTransferManager();
    pthread_attr_t * getPthreadAttr();
    sem_t * getListRefreshSem();
    int ctimeMSec();
};
