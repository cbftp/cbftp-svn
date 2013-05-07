#pragma once

#include <openssl/ssl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <iostream>
#include <map>

#include "workmanager.h"
#include "globalcontext.h"

class EventReceiver;

#define MAXEVENTS 32

extern GlobalContext * global;

class IOManager {
private:
  pthread_t thread;
  std::map<int, int> typemap;
  std::map<int, EventReceiver *> receivermap;
  std::map<int, SSL *> sslmap;
  static void * run(void *);
  WorkManager * wm;
  int epollfd;
public:
  IOManager();
  void runInstance();
  void registerStdin(EventReceiver *);
  int registerTCPClientSocket(EventReceiver *, std::string, int);
  void negotiateSSL(int);
  int registerUDPServerSocket(EventReceiver *, int);
  void send(int, std::string);
  void closeSocket(int);
};
