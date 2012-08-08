#pragma once

#include <list>
#include <pthread.h>
#include <semaphore.h>

#include "commandqueueelement.h"

class FileList;

class FTPThreadCom {
  private:
    std::list<CommandQueueElement *> commandqueue;
    pthread_mutex_t commandqueue_mutex;
    sem_t * notifysem;
  public:
    FTPThreadCom(sem_t *);
    void loginSuccessful(int);
    CommandQueueElement * getCommand();
    void commandProcessed();
    void loginConnectFailed(int);
    void loginUnknownResponse(int);
    void loginUserDenied(int, int, char *);
    void loginTLSFailed(int, int);
    void loginPasswordDenied(int, int, char *);
    void loginKillFailed(int, int, char *);
    void connectionClosedUnexpectedly(int);
    void fileListUpdated(int);
    void fileListRetrieved(int, FileList *);
    void putCommand(int, int);
    void putCommand(int, int, int);
    void putCommand(int, int, int, char *);
    void putCommand(int, int, void *, void *);
};
