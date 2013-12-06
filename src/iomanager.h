#pragma once

#include <pthread.h>
#include <map>
#include <string>
#include <openssl/ssl.h>
#include <list>

class EventReceiver;
class DataBlockPool;
class WorkManager;
class GlobalContext;
class DataBlock;

#define MAXEVENTS 32

#define FD_UNUSED 1030
#define FD_KEYBOARD 1031
#define FD_TCP_PLAIN 1032
#define FD_TCP_SSL_NEG_REDO_CONN 1033
#define FD_TCP_SSL_NEG_REDO_WRITE 1034
#define FD_TCP_SSL 1035
#define FD_UDP 1036
#define FD_TCP_SERVER 1037
#define FD_TCP_PLAIN_LISTEN 1038
#define FD_TCP_SSL_NEG_REDO_ACCEPT 1039

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
  std::string defaultinterface;
  std::string getInterfaceAddress(std::string);
  bool hasdefaultinterface;
public:
  IOManager();
  void runInstance();
  void registerStdin(EventReceiver *);
  int registerTCPClientSocket(EventReceiver *, std::string, int);
  int registerTCPServerSocket(EventReceiver *, int);
  int registerTCPServerSocket(EventReceiver *, int, bool);
  void registerTCPServerClientSocket(EventReceiver *, int);
  void negotiateSSL(int);
  int registerUDPServerSocket(EventReceiver *, int);
  void sendData(int, std::string);
  void sendData(int, char *, unsigned int);
  std::string getCipher(int);
  std::string getSocketAddress(int);
  void closeSocket(int);
  std::list<std::pair<std::string, std::string> > listInterfaces();
  void readConfiguration();
  void writeState();
  std::string getDefaultInterface();
  void setDefaultInterface(std::string);
  bool hasDefaultInterface();
};
