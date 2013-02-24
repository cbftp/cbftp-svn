#pragma once

#include <stdlib.h>
#include <list>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <cerrno>
#include <openssl/ssl.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include "filelist.h"
#include "ftpthreadcom.h"
#include "site.h"
#include "siterace.h"
#include "globalcontext.h"
#include "commandqueueelement.h"
#include "rawbuffer.h"

extern GlobalContext * global;

#define MAXDATASIZE 2048
#define RAWBUFMAXLEN 1024

extern SSL_CTX * ssl_ctx;

class FTPThread {
  private:
    RawBuffer * rawbuf;
    struct addrinfo sock, *res;
    int id;
    std::string status;
    Site * site;
    FTPThreadCom * ftpthreadcom;
    pthread_t thread;
    sem_t commandsem;
    sem_t transfersem;
    int transferstatus;
    std::list<CommandQueueElement *> commandqueue;
    pthread_mutex_t commandq_mutex;
    bool controlssl;
    int sockfd;
    SSL * ssl;
    bool aborted;
    std::string currentpath;
    struct timeval tv;
    struct timeval tvsocket;
    fd_set readfd;
    bool protectedmode;
    int write(const char *);
    int write(const char *, bool);
    int read();
    int readall(char **, bool);
    int readallsub(char **, char *, bool);
    bool pendingread();
    static void * run(void *);
  public:
    int getId();
    void setId(int);
    std::string getStatus();
    void loginAsync();
    bool loginT();
    void reconnectAsync();
    void reconnectT();
    void doUSERPASSAsync();
    void doUSERPASST(bool);
    void loginKillAsync();
    void loginKillT();
    FTPThread(int, Site *, FTPThreadCom *);
    int updateFileList(FileList *);
    std::string getCurrentPath();
    std::string doPWD();
    void setProtectedModeAsync();
    void unsetProtectedModeAsync();
    void doPROTPT();
    void doPROTCT();
    bool doCPSV(std::string **);
    void doCPSVT(std::string **, sem_t *);
    bool doPASV(std::string **);
    bool doPASV(std::string **, bool);
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
    bool awaitTransferComplete();
    void doQUITAsync();
    void doQUITT();
    void disconnectAsync();
    void disconnectT();
    void refreshRaceFileListAsync(SiteRace *);
    void refreshRaceFileListT(SiteRace *);
    void getFileListAsync(std::string);
    void getFileListT(std::string);
    std::list<CommandQueueElement *> * getCommandQueue();
    void runInstance();
    void putCommand(CommandQueueElement *);
    RawBuffer * getRawBuffer();
};
