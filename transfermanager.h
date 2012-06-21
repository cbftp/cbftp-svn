#pragma once

#include <string>
#include <pthread.h>
#include <semaphore.h>

#include "globalcontext.h"
#include "sitethread.h"
#include "scoreboardelement.h"
#include "transfer.h"

#define MAX_THREADS 256

extern GlobalContext * global;

class TransferManager {
  private:
    int inuse;
    pthread_mutex_t inuse_mutex;
    pthread_t threadid[MAX_THREADS];
    sem_t transfersem;
    sem_t transfercheckoutsem;
    Transfer * transfertmp;
    int threads;
    std::vector<Transfer *> transfers;
  public:
    TransferManager();
    void newTransfer(ScoreBoardElement *);
    void runTransferInstance();
};

void * runTransfer(void *);
