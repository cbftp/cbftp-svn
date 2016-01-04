#pragma once

#include <map>
#include <string>
#include <list>

#include "eventreceiver.h"
#include "socketinfo.h"
#include "lock.h"
#include "polling.h"
#include "pointer.h"
#include "threading.h"

class DataBlockPool;
class WorkManager;
class GlobalContext;
class ScopeLock;

#define TICKPERIOD 100
#define TIMEOUT_MS 5000
#define MAX_SEND_BUFFER 1048576

extern GlobalContext * global;

class IOManager : private EventReceiver {
private:
  Polling polling;
  Thread<IOManager> thread;
  mutable Lock socketinfomaplock;
  std::map<int, SocketInfo> socketinfomap;
  std::map<int, int> connecttimemap;
  std::map<int, int> sockfdidmap;
  WorkManager * wm;
  DataBlockPool * blockpool;
  Pointer<DataBlockPool> sendblockpool;
  int blocksize;
  int sockidcounter;
  std::string defaultinterface;
  std::string getInterfaceAddress(std::string);
  bool handleError(EventReceiver *);
  bool investigateSSLError(int, int, int);
  bool hasdefaultinterface;
  void closeSocketIntern(int);
  void handleKeyboardIn(SocketInfo &, ScopeLock &);
  void handleTCPConnectingOut(SocketInfo &);
  void handleTCPPlainIn(SocketInfo &);
  void handleTCPPlainOut(SocketInfo &);
  void handleTCPSSLNegotiationIn(SocketInfo &);
  void handleTCPSSLNegotiationOut(SocketInfo &);
  void handleTCPSSLIn(SocketInfo &);
  void handleTCPSSLOut(SocketInfo &);
  void handleUDPIn(SocketInfo &);
  void handleTCPServerIn(SocketInfo &);
public:
  IOManager();
  void init();
  void run();
  void registerStdin(EventReceiver *);
  void tick(int);
  int registerTCPClientSocket(EventReceiver *, std::string, int, int *);
  int registerTCPServerSocket(EventReceiver *, int);
  int registerTCPServerSocket(EventReceiver *, int, bool);
  void registerTCPServerClientSocket(EventReceiver *, int);
  void adopt(EventReceiver *, int);
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
  std::string getDefaultInterface() const;
  void setDefaultInterface(std::string);
  bool hasDefaultInterface() const;
};
