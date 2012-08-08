#include "ftpthreadcom.h"

FTPThreadCom::FTPThreadCom(sem_t * notifysem) {
  pthread_mutex_init(&commandqueue_mutex, NULL);
  this->notifysem = notifysem;
}

void FTPThreadCom::loginSuccessful(int id) {
  putCommand(id, 0);
}

CommandQueueElement * FTPThreadCom::getCommand() {
  pthread_mutex_lock(&commandqueue_mutex);
  CommandQueueElement * cqe = commandqueue.front();
  pthread_mutex_unlock(&commandqueue_mutex);
  return cqe;
}

void FTPThreadCom::commandProcessed() {
  pthread_mutex_lock(&commandqueue_mutex);
  delete commandqueue.front();
  commandqueue.pop_front();
  pthread_mutex_unlock(&commandqueue_mutex);
}

void FTPThreadCom::loginConnectFailed(int id) {
  putCommand(id, 1);
}

void FTPThreadCom::loginTLSFailed(int id, int status) {
  putCommand(id, 5, status);
}

void FTPThreadCom::loginUnknownResponse(int id) {
  putCommand(id, 2);
}

void FTPThreadCom::loginUserDenied(int id, int status, char * reply) {
  putCommand(id, 3, status, reply);
}

void FTPThreadCom::loginPasswordDenied(int id, int status, char * reply) {
  putCommand(id, 4, status, reply);
}

void FTPThreadCom::loginKillFailed(int id, int status, char * reply) {
  putCommand(id, 7, status, reply);
}

void FTPThreadCom::connectionClosedUnexpectedly(int id) {
  putCommand(id, 6);
}

void FTPThreadCom::fileListUpdated(int id) {
  putCommand(id, 10);
}

void FTPThreadCom::fileListRetrieved(int id, FileList * filelist) {
  int * idp = new int(id);
  pthread_mutex_lock(&commandqueue_mutex);
  commandqueue.push_back(new CommandQueueElement(11, (void *) idp, (void *) filelist, (void *) NULL));
  pthread_mutex_unlock(&commandqueue_mutex);
  sem_post(notifysem);
}

void FTPThreadCom::putCommand(int id, int cid) {
  putCommand(id, cid, 0, NULL);
}

void FTPThreadCom::putCommand(int id, int cid, int status) {
  putCommand(id, cid, status, NULL);
}

void FTPThreadCom::putCommand(int id, int cid, int status, char * reply) {
  int * statusp = new int(status);
  putCommand(id, cid, (void *) statusp, (void *) reply);
}

void FTPThreadCom::putCommand(int id, int cid, void * arg3, void * arg4) {
  int * idp = new int(id);
  pthread_mutex_lock(&commandqueue_mutex);
  commandqueue.push_back(new CommandQueueElement(cid, (void *) idp, arg3, arg4));
  pthread_mutex_unlock(&commandqueue_mutex);
  sem_post(notifysem);
}
