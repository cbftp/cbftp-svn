#pragma once

#include <openssl/ssl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <iostream>
#include <map>

#include "workmanager.h"
#include "globalcontext.h"
#include "datablock.h"
#include "datablockpool.h"

class EventReceiver;

#define MAXEVENTS 32

#define FD_UNUSED 1030
#define FD_KEYBOARD 1031
#define FD_TCP_PLAIN 1032
#define FD_TCP_SSL_NEG_REDO_CONN 1033
#define FD_TCP_SSL_NEG_REDO_WRITE 1034
#define FD_TCP_SSL 1035
#define FD_UDP 1036
#define FD_TCP_SERVER 1037

extern GlobalContext * global;

class IOManager {
private:
  pthread_t thread;
  std::map<int, int> typemap;
  std::map<int, EventReceiver *> receivermap;
  std::map<int, SSL *> sslmap;
  std::map<int, std::list<DataBlock> > sendqueuemap;
  std::map<int, std::string> addrmap;
  static void * run(void *);
  WorkManager * wm;
  int epollfd;
  DataBlockPool * blockpool;
  int blocksize;
public:
  IOManager();
  void runInstance();
  void registerStdin(EventReceiver *);
  int registerTCPClientSocket(EventReceiver *, std::string, int);
  int registerTCPServerSocket(EventReceiver *, int);
  void negotiateSSL(int);
  int registerUDPServerSocket(EventReceiver *, int);
  void sendData(int, std::string);
  void sendData(int, char *, unsigned int);
  std::string getCipher(int);
  std::string getSocketAddress(int);
  void closeSocket(int);
};
