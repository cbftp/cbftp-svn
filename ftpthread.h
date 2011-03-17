#pragma once
#include <iostream>
#include <stdlib.h>
#include <list>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <cerrno>
#include <openssl/ssl.h>
#include <pthread.h>
#include <semaphore.h>
#include "filelist.h"
#include "ftpthreadcom.h"
#include "site.h"
#include "siterace.h"
#include "globalcontext.h"
#include "commandqueueelement.h"

extern GlobalContext * global;

#define MAXDATASIZE 2048

//minimum sleep delay (between refreshes / hammer attempts) in ms
#define SLEEPDELAY 150

extern SSL_CTX * ssl_ctx;

class FTPThread {
  private:
    struct addrinfo sock, *res;
    int id;
    Site * site;
    FTPThreadCom * ftpthreadcom;
    pthread_t thread;
    pthread_t tickthread;
    sem_t commandsem;
    sem_t transfersem;
    sem_t tick;
    sem_t * list_refresh;
    std::list<CommandQueueElement *> commandqueue;
    pthread_mutex_t commandq_mutex;
    bool controlssl;
    int sockfd;
    SSL * ssl;
    bool ready;
    bool refreshloop;
    bool aborted;
    std::string currentpath;
    SiteRace * currentsiterace;
    struct timeval tv;
    fd_set readfd;
    int write(const char *);
    int write(const char *, bool);
    int read();
    int readall(char **, bool);
    int readallsub(char **, char *, bool);
    bool pendingread();
  public:
    void loginAsync();
    bool loginT();
    void reconnectAsync();
    void reconnectT();
    void doUSERPASSAsync();
    void doUSERPASST(bool);
    void loginKillAsync();
    void loginKillT();
    FTPThread(int, Site *, FTPThreadCom *);
    bool isReady();
    void setBusy();
    void setReady();
    void refreshLoopAsync(SiteRace *);
    void refreshLoopT();
    int updateFileList(FileList *);
    std::string getCurrentPath();
    std::string doPWD();
    void sleepTickAsync();
    void sleepTickT();
    bool doPASV(std::string **);
    void doPASVT(std::string **, sem_t *);
    bool doPORT(std::string);
    void doPORTT(std::string, sem_t *);
    void doCWDAsync(std::string);
    bool doCWDT(std::string);
    bool doMKDirT(std::string);
    void doCWDorMKDirAsync(std::string, std::string);
    void doCWDorMKDirT(std::string, std::string);
    bool doPRETRETR(std::string);
    void doPRETRETRT(std::string, int *, sem_t *);
    bool doRETR(std::string);
    bool doRETRAsyncT(std::string, int *, sem_t *);
    bool doPRETSTOR(std::string);
    void doPRETSTORT(std::string, int *, sem_t *);
    bool doSTOR(std::string);
    bool doSTORAsyncT(std::string, int *, bool *, sem_t *);
    void abortTransfer();
    void awaitTransferComplete();
    void doQUITAsync();
    void doQUITT();
    void disconnectAsync();
    void disconnectT();
    std::list<CommandQueueElement *> * getCommandQueue();
    static void * run(void *);
    static void * runTick(void *);
    void runInstance();
    void runTickInstance();
    void postTick();
    void putCommand(CommandQueueElement *);
    void putCommand(CommandQueueElement *, bool);
};
