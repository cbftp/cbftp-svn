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
class TickPoke;
class ScopeLock;
class Logger;

class IOManager : public EventReceiver {
private:
  Polling polling;
  Thread<IOManager> thread;
  mutable Lock socketinfomaplock;
  std::map<int, SocketInfo> socketinfomap;
  std::map<int, int> connecttimemap;
  std::map<int, int> sockfdidmap;
  WorkManager * wm;
  TickPoke * tp;
  DataBlockPool * blockpool;
  Pointer<DataBlockPool> sendblockpool;
  int blocksize;
  int sockidcounter;
  Pointer<Logger> logger;
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
  void handleTCPNameResolution(SocketInfo &);
  void log(const std::string &);
public:
  IOManager(WorkManager *, TickPoke *);
  void init();
  void run();
  void registerStdin(EventReceiver *);
  void tick(int);
  int registerTCPClientSocket(EventReceiver *, std::string, int);
  int registerTCPClientSocket(EventReceiver *, std::string, int, bool &);
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
  void resolveDNS(int);
  void asyncTaskComplete(int, int);
  std::list<std::pair<std::string, std::string> > listInterfaces();
  std::string getDefaultInterface() const;
  void setDefaultInterface(std::string);
  bool hasDefaultInterface() const;
  void setLogger(Pointer<Logger>);
};
