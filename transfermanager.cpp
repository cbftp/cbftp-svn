#include "transfermanager.h"

TransferManager::TransferManager() {
  threads = 0;
  inuse = 0;
  sem_init(&transfersem, 0, 0);
  sem_init(&transfercheckoutsem, 0, 0);
  pthread_mutex_init(&inuse_mutex, NULL);
  for (int i = 0; i < 10; i++) {
    pthread_create(&threadid[threads++], global->getPthreadAttr(), runTransfer, (void *) this);
  }
}

void TransferManager::newTransfer(ScoreBoardElement * sbe) {
  std::string name = sbe->fileName();
  SiteThread * src = sbe->getSource();
  SiteThread * dst = sbe->getDestination();
  SiteRace * srs = sbe->getSourceSiteRace();
  SiteRace * srd = sbe->getDestinationSiteRace();
  transfertmp = new Transfer(name, src, srs, dst, srd);
  transfers.push_back(transfertmp);
  if (inuse >= threads-1 && threads < MAX_THREADS) {
    pthread_create(&threadid[threads++], global->getPthreadAttr(), runTransfer, (void *) this);
  }
  sem_post(&transfersem);
  sem_wait(&transfercheckoutsem);
}

void * runTransfer(void * arg) {
  ((TransferManager *) arg)->runTransferInstance();
}

void TransferManager::runTransferInstance() {
  while(1) {
    sem_wait(&transfersem);
    pthread_mutex_lock(&inuse_mutex);
    inuse++;
    pthread_mutex_unlock(&inuse_mutex);
    Transfer * t = transfertmp;
    sem_post(&transfercheckoutsem);
    t->run();
    delete t;
    pthread_mutex_lock(&inuse_mutex);
    inuse--;
    pthread_mutex_unlock(&inuse_mutex);
  }
}
