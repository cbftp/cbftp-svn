#pragma once

#include <pthread.h>
#include <map>
#include <string>
#include <openssl/ssl.h>
#include <list>

#include "eventreceiver.h"
#include "socketinfo.h"

class DataBlockPool;
class WorkManager;
class GlobalContext;

#define MAXEVENTS 32

#define TICKPERIOD 100
#define TIMEOUT_MS 5000

extern GlobalContext * global;

class IOManager : private EventReceiver {
private:
  pthread_t thread;
  pthread_mutex_t socketinfolock;
  std::map<int, SocketInfo> socketinfo;
  std::map<int, int> connecttimemap;
  static void * run(void *);
  WorkManager * wm;
  int epollfd;
  DataBlockPool * blockpool;
  int blocksize;
  std::string defaultinterface;
  std::string getInterfaceAddress(std::string);
  void negotiateSSL(int, EventReceiver *);
  bool investigateSSLError(int, int, int);
  bool hasdefaultinterface;
  void closeSocketIntern(int);
public:
  IOManager();
  void runInstance();
  void registerStdin(EventReceiver *);
  void tick(int);
  int registerTCPClientSocket(EventReceiver *, std::string, int, int *);
  int registerTCPServerSocket(EventReceiver *, int);
  int registerTCPServerSocket(EventReceiver *, int, bool);
  void registerTCPServerClientSocket(EventReceiver *, int);
  void negotiateSSLConnect(int);
  void negotiateSSLConnect(int, EventReceiver *);
  void negotiateSSLAccept(int);
  void forceSSLhandshake(int);
  int registerUDPServerSocket(EventReceiver *, int);
  void sendData(int, std::string);
  void sendData(int, const char *, unsigned int);
  std::string getCipher(int);
  std::string getSocketAddress(int) const;
  void closeSocket(int);
  std::list<std::pair<std::string, std::string> > listInterfaces();
  void readConfiguration();
  void writeState();
  std::string getDefaultInterface() const;
  void setDefaultInterface(std::string);
  bool hasDefaultInterface() const;
};
