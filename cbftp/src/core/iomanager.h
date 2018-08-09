#pragma once

#include <map>
#include <memory>
#include <string>
#include <list>
#include <set>
#include <openssl/ssl.h>
#include <unordered_map>

#include "eventreceiver.h"
#include "socketinfo.h"
#include "lock.h"
#include "polling.h"
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
  std::set<int> autopaused;
  std::set<int> manuallypaused;
  WorkManager * wm;
  TickPoke * tp;
  DataBlockPool * blockpool;
  std::shared_ptr<DataBlockPool> sendblockpool;
  int blocksize;
  int sockidcounter;
  std::shared_ptr<Logger> logger;
  std::string defaultinterface;
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
  void pollRead(SocketInfo &);
  void pollWrite(SocketInfo &);
  void setPollRead(SocketInfo &);
  void setPollWrite(SocketInfo &);
  void unsetPoll(SocketInfo &);
  void autoPause(SocketInfo &);
public:
  IOManager(WorkManager *, TickPoke *);
  void init();
  void run();
  void registerStdin(EventReceiver *);
  void tick(int);
  int registerTCPClientSocket(EventReceiver *, const std::string &, int);
  int registerTCPClientSocket(EventReceiver *, const std::string &, int, bool &, bool);
  int registerTCPServerSocket(EventReceiver *, int);
  int registerTCPServerSocket(EventReceiver *, int, bool);
  void registerTCPServerClientSocket(EventReceiver *, int);
  void registerTCPServerClientSocket(EventReceiver *, int, bool);
  void adopt(EventReceiver *, int);
  void negotiateSSLConnect(int);
  void negotiateSSLConnect(int, int);
  void negotiateSSLAccept(int);
  int registerUDPServerSocket(EventReceiver *, int);
  bool sendData(int, const std::string &);
  bool sendData(int, const char *, unsigned int);
  std::string getCipher(int) const;
  bool getSSLSessionReused(int) const;
  std::string getSocketAddress(int) const;
  int getSocketPort(int) const;
  std::string getInterfaceAddress(int) const;
  void closeSocket(int);
  void resolveDNS(int);
  void asyncTaskComplete(int, int);
  std::list<std::pair<std::string, std::string> > listInterfaces();
  std::string getDefaultInterface() const;
  void setDefaultInterface(const std::string &);
  bool hasDefaultInterface() const;
  std::string getInterfaceAddress(const std::string &);
  void setLogger(std::shared_ptr<Logger>);
  void workerReady();
  void pause(int);
  void resume(int);
};
