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

void FTPThreadCom::putCommand(int id, int cid) {
  putCommand(id, cid, 0, NULL);
}

void FTPThreadCom::putCommand(int id, int cid, int status) {
  putCommand(id, cid, status, NULL);
}

void FTPThreadCom::putCommand(int id, int cid, int status, char * reply) {
  int * idp = new int(id);
  int * statusp = new int(status);
  pthread_mutex_lock(&commandqueue_mutex);
  commandqueue.push_back(new CommandQueueElement(cid, (void *) idp, (void *) statusp, (void *) reply));
  pthread_mutex_unlock(&commandqueue_mutex);
  sem_post(notifysem);
}
