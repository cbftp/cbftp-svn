#pragma once

#include <pthread.h>
#include <map>
#include <string>
#include <openssl/ssl.h>
#include <list>

#include "eventreceiver.h"
#include "socketinfo.h"
#include "lock.h"
#include "pointer.h"

class DataBlockPool;
class WorkManager;
class GlobalContext;
class Polling;

#define TICKPERIOD 100
#define TIMEOUT_MS 5000

extern GlobalContext * global;

class IOManager : private EventReceiver {
private:
  pthread_t thread;
  Pointer<Polling> polling;
  mutable Lock socketinfomaplock;
  std::map<int, SocketInfo> socketinfomap;
  std::map<int, int> connecttimemap;
  std::map<int, int> sockfdidmap;
  static void * run(void *);
  WorkManager * wm;
  DataBlockPool * blockpool;
  int blocksize;
  int sockidcounter;
  std::string defaultinterface;
  std::string getInterfaceAddress(std::string);
  bool handleError(EventReceiver *);
  void negotiateSSL(int, EventReceiver *);
  bool investigateSSLError(int, int, int);
  bool hasdefaultinterface;
  void closeSocketIntern(int);
  static const char * getCipher(SSL *);
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
  std::string getCipher(int) const;
  std::string getSocketAddress(int) const;
  void closeSocket(int);
  std::list<std::pair<std::string, std::string> > listInterfaces();
  void readConfiguration();
  void writeState();
  std::string getDefaultInterface() const;
  void setDefaultInterface(std::string);
  bool hasDefaultInterface() const;
};
