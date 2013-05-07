#pragma once

#include <string>
#include <vector>
#include <list>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <poll.h>

#include "globalcontext.h"
#include "datafilehandler.h"
#include "engine.h"
#include "eventreceiver.h"
#include "iomanager.h"

#define DEFAULTPORT 55477
#define DEFAULTPASS "DEFAULT"

extern GlobalContext * global;

class RemoteCommandHandler : private EventReceiver {
private:
  bool enabled;
  std::string password;
  int port;
  int sockfd;
  pthread_t thread;
  sem_t commandsem;
  std::list<int> commandqueue;
  pthread_mutex_t commandq_mutex;
  void connect();
  void disconnect();
  void handleMessage(std::string);
public:
  RemoteCommandHandler();
  bool isEnabled();
  int getUDPPort();
  std::string getPassword();
  void setPassword(std::string);
  void setPort(int);
  void setEnabled(bool);
  void readConfiguration();
  void writeState();
  void FDData(char *, unsigned int);
};
