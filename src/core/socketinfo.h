#pragma once

#include <string>
#include <list>
#include <openssl/ssl.h>

#include "datablock.h"
#include "prio.h"

enum SocketType {
  FD_UNUSED,
  FD_KEYBOARD,
  FD_TCP_PLAIN,
  FD_TCP_SSL,
  FD_UDP,
  FD_TCP_SERVER,
  FD_TCP_PLAIN_LISTEN,
  FD_TCP_SSL_NEG_CONNECT,
  FD_TCP_SSL_NEG_REDO_CONNECT,
  FD_TCP_SSL_NEG_ACCEPT,
  FD_TCP_SSL_NEG_REDO_ACCEPT,
  FD_TCP_SSL_NEG_REDO_HANDSHAKE,
  FD_TCP_CONNECTING,
  FD_TCP_RESOLVING
};

enum Direction {
  DIR_IN,
  DIR_OUT
};

class EventReceiver;

class SocketInfo {
public:
  SocketInfo() : type(FD_UNUSED), fd(0), id(0), port(0), localport(0), gairet(0),
                 gaires(NULL), gaiasync(false), receiver(NULL), ssl(NULL),
                 prio(PRIO_NORMAL), paused(false), direction(DIR_IN)
  {
  }
  SocketType type;
  int fd;
  int id;
  std::string addr;
  int port;
  std::string localaddr;
  int localport;
  int gairet;
  struct addrinfo * gaires;
  std::string gaierr;
  bool gaiasync;
  EventReceiver * receiver;
  mutable std::list<DataBlock> sendqueue;
  SSL * ssl;
  Prio prio;
  bool paused;
  int direction;
};
